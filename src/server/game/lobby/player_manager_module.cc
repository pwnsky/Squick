#include "player_manager_module.h"
#include <server/db_proxy/logic/common_redis_module.h>

namespace game::player {
	bool PlayerManagerModule::Start()
	{
		m_pElementModule = pPluginManager->FindModule<IElementModule>();
		m_pClassModule = pPluginManager->FindModule<IClassModule>();
		m_pNetModule = pPluginManager->FindModule<INetModule>();
		m_pKernelModule = pPluginManager->FindModule<IKernelModule>();
		m_pGameToDBModule = pPluginManager->FindModule<IGameServerToDBModule>();
		m_pLogModule = pPluginManager->FindModule<ILogModule>();

		m_pGameServerNet_ServerModule = pPluginManager->FindModule<IGameServerNet_ServerModule>();
		m_pNetClientModule = pPluginManager->FindModule<INetClientModule>();
		m_pScheduleModule = pPluginManager->FindModule<IScheduleModule>();
		m_pDataTailModule = pPluginManager->FindModule<IDataTailModule>();
		m_pSceneModule = pPluginManager->FindModule<ISceneModule>();

		m_pEventModule = pPluginManager->FindModule<IEventModule>();
		m_pRoomModule = pPluginManager->FindModule<IRoomModule>();

		m_pGameplayManagerModule = pPluginManager->FindModule<play::IGameplayManagerModule>();

		return true;
	}

	bool PlayerManagerModule::AfterStart()
	{
		m_pKernelModule->AddClassCallBack(excel::Player::ThisName(), this, &PlayerManagerModule::OnPlayerObjectEvent);
		return true;
	}

	bool PlayerManagerModule::ReadyUpdate()
	{
		m_pNetModule->AddReceiveCallBack(SquickStruct::GameLobbyRPC::REQ_ENTER, this, &PlayerManagerModule::OnReqPlayerEnter);
		m_pNetModule->AddReceiveCallBack(SquickStruct::GameLobbyRPC::REQ_LEAVE, this, &PlayerManagerModule::OnReqPlayerLeave);
		m_pNetClientModule->AddReceiveCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_DB, SquickStruct::DbRPC::ACK_PLAYER_DATA_LOAD, this, &PlayerManagerModule::OnAckPlayerDataLoad);

		return true;
	}

	void PlayerManagerModule::OnReqPlayerEnter(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
	{
		Guid clientID;
		SquickStruct::ReqEnter xMsg;
		if (!m_pNetModule->ReceivePB(msgID, msg, len, xMsg, clientID)) {
			return;
		}

		dout << "?????????????????? " << clientID.ToString() << std::endl;
		
		SQUICK_SHARE_PTR<IGameServerNet_ServerModule::GateServerInfo> pGateServerinfo = m_pGameServerNet_ServerModule->GetGateServerInfoBySockIndex(sockIndex);
		if (nullptr == pGateServerinfo) {
			return;
		}

		int gateID = -1;
		if (pGateServerinfo->xServerData.pData) {
			gateID = pGateServerinfo->xServerData.pData->server_id();
		}
		
		if (gateID < 0) {
			return;
		}

		// ?????????
		if (!m_pGameServerNet_ServerModule->AddPlayerGateInfo(clientID, clientID, gateID)) {
			return;
		}

		m_pNetClientModule->SendBySuitWithOutHead(SQUICK_SERVER_TYPES::SQUICK_ST_DB, sockIndex, SquickStruct::DbRPC::REQ_PLAYER_DATA_LOAD, std::string(msg, len));
	}

	// ??????????????????
	void PlayerManagerModule::OnAckPlayerDataLoad(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len) {
		dout << "??????????????????\n";
		Guid clientID;
		SquickStruct::PlayerData xMsg;
		if (!m_pNetModule->ReceivePB(msgID, msg, len, xMsg, clientID)) {
			return;
		}
		
		// ?????????????????????????????????
		// ??????????????????????????????ObjectID
		//Guid objectID = INetModule::ProtobufToStruct(xMsg.object());
		Guid objectID = clientID;


		mxObjectDataCache[objectID] = xMsg; //??????????????????

		Player* player = nullptr;
		// ??????????????????????????????????????????????????????
		if (m_players.find(objectID) == m_players.end()) {
			player = new Player();
			m_players[objectID] = player;
		}

		player = m_players[objectID];
		// ??????????????????????????????????????????
		player->OnEnterGame();
		player->loginTime = time(nullptr);

		if (m_pKernelModule->GetObject(objectID)) { // ???????????????????????????
			//it should be rebind with proxy's netobject
			m_pKernelModule->DestroyObject(objectID);
		}

		/*
		DataList var;
		SQUICK_SHARE_PTR<IGameServerNet_ServerModule::GateBaseInfo>  pGateInfo = m_pGameServerNet_ServerModule->GetPlayerGateInfo(clientID);
		if (nullptr == pGateInfo)
		{
			dout << "Error to load GateBaseInfo: ClientID: " << clientID.ToString() << std::endl;
			return;
		}

		var.AddString(SquickProtocol::Player::GateID());
		var.AddInt(pGateInfo->gateID);

		var.AddString(SquickProtocol::Player::GameID());
		var.AddInt(pPluginManager->GetAppID());

		var.AddString(SquickProtocol::Player::Connection());
		var.AddInt(1);

		SQUICK_SHARE_PTR<IObject> pObject = m_pKernelModule->CreateObject(objectID, 1, 0, SquickProtocol::Player::ThisName(), "", var);
		*/
	
		
		SquickStruct::AckEnter ack;
		ack.set_code(0);
		*ack.mutable_client() = INetModule::StructToProtobuf(clientID);
		*ack.mutable_object() = INetModule::StructToProtobuf(objectID);

		m_pGameServerNet_ServerModule->SendMsgPBToGate(SquickStruct::ACK_ENTER, ack, clientID);
	}

	// ??????????????????
	int PlayerManagerModule::OnPlayerObjectEvent(const Guid& self, const std::string& className, const CLASS_OBJECT_EVENT classEvent, const DataList& var) {
		// ??????
		if (CLASS_OBJECT_EVENT::COE_DESTROY == classEvent) {
			//m_pDataTailModule->LogObjectData(self);
			// ????????????
			dout << "????????????????????????: " << self.ToString() << " \n";
			m_pKernelModule->SetPropertyInt(self, excel::Player::LastOfflineTime(), SquickGetTimeS());
			SaveDataToDb(self); // ????????????????????????

			Player* player = m_players[self];
			if (player == nullptr) {
				dout << "Player offline not found!\n";
				return -1;
			}

			player->OnOffline();
			player->offlineTime = time(nullptr); // ????????????????????????Player Manager????????????????????????????????????
			m_offlineCachePlayers[self] = player;
		}
		else if (CLASS_OBJECT_EVENT::COE_CREATE_LOADDATA == classEvent) {
			dout << "????????????????????????\n";
			//m_pDataTailModule->StartTrail(self);
			//m_pDataTailModule->LogObjectData(self);

			LoadDataFromDb(self);
			m_pKernelModule->SetPropertyInt(self, excel::Player::OnlineTime(), SquickGetTimeS());
		}
		else if (CLASS_OBJECT_EVENT::COE_CREATE_FINISH == classEvent) {
			dout << "????????????????????????\n";
			auto it = mxObjectDataCache.find(self);
			if (it != mxObjectDataCache.end())
			{
				mxObjectDataCache.erase(it);
			}

			// ???3?????? ????????????????????????????????????
			// m_pScheduleModule->AddSchedule(self, "SaveDataOnTime", this, &PlayerManagerModule::SaveDataOnTime, 180.0f, -1);
		}
		return 0;
	}

	// ??????????????????????????????
	void PlayerManagerModule::LoadDataFromDb(const Guid& self) {
		auto it = mxObjectDataCache.find(self);
		if (it != mxObjectDataCache.end()) {
			SQUICK_SHARE_PTR<IObject> xObject = m_pKernelModule->GetObject(self);
			if (xObject) {
				SQUICK_SHARE_PTR<IPropertyManager> xPropManager = xObject->GetPropertyManager();
				SQUICK_SHARE_PTR<IRecordManager> xRecordManager = xObject->GetRecordManager();

				if (xPropManager) {
					CommonRedisModule::ConvertPBToPropertyManager(it->second.property(), xPropManager);
				}

				if (xRecordManager) {
					CommonRedisModule::ConvertPBToRecordManager(it->second.record(), xRecordManager);
				}
				mxObjectDataCache.erase(it);
				xObject->SetPropertyInt(excel::Player::GateID(), pPluginManager->GetAppID());
				auto playerGateInfo = m_pGameServerNet_ServerModule->GetPlayerGateInfo(self);
				if (playerGateInfo) {
					xObject->SetPropertyInt(excel::Player::GateID(), playerGateInfo->gateID);
				}
			}
		}
	}

	// ??????????????????????????????
	void PlayerManagerModule::SaveDataToDb(const Guid& self) {
		SQUICK_SHARE_PTR<IObject> xObject = m_pKernelModule->GetObject(self);
		if (xObject) {
			SQUICK_SHARE_PTR<IPropertyManager> xPropManager = xObject->GetPropertyManager();
			SQUICK_SHARE_PTR<IRecordManager> xRecordManager = xObject->GetRecordManager();
			SquickStruct::PlayerData xDataPack;

			*xDataPack.mutable_object() = INetModule::StructToProtobuf(self);

			*(xDataPack.mutable_property()->mutable_player_id()) = INetModule::StructToProtobuf(self);
			*(xDataPack.mutable_record()->mutable_player_id()) = INetModule::StructToProtobuf(self);

			if (xPropManager) {
				CommonRedisModule::ConvertPropertyManagerToPB(xPropManager, xDataPack.mutable_property(), false, true);
			}

			if (xRecordManager) {
				CommonRedisModule::ConvertRecordManagerToPB(xRecordManager, xDataPack.mutable_record(), false, true);
			}
			m_pNetClientModule->SendSuitByPB(SQUICK_SERVER_TYPES::SQUICK_ST_DB, self.GetData(), SquickStruct::DbRPC::REQ_PLAYER_DATA_SAVE, xDataPack);
		}
	}


	// ????????????????????????
	int PlayerManagerModule::SaveDataOnTime(const Guid& self, const std::string& name, const float fIntervalTime, const int count) {
		SaveDataToDb(self);
		return 0;
	}

	bool PlayerManagerModule::Destory() {
		return true;
	}

	bool PlayerManagerModule::Update() {
		dout << "Update...\n";
		return true;
	}

	// ????????????????????????????????????player.cc??????
	void PlayerManagerModule::OnSendToClient(const uint16_t msgID, google::protobuf::Message& xMsg, const Guid& client_id) {
		m_pGameServerNet_ServerModule->SendMsgPBToGate(msgID, xMsg, client_id);
	}

	// virtual  ();
	// ????????????????????????????????????player.cc??????
	Player* PlayerManagerModule::GetPlayer(const Guid& clientID) {
		return m_players[clientID];
	}

	int PlayerManagerModule::GetPlayerRoomID(const Guid& clientID) {
		auto player = m_players[clientID];
		if (player != nullptr) {
			return player->GetRoomID();
		}
		return -1;
	}

	void PlayerManagerModule::SetPlayerRoomID(const Guid& clientID, int groupID) {
		auto player = m_players[clientID];
		if (player != nullptr) {
			player->SetRoomID(groupID);
		}
	}

	int PlayerManagerModule::GetPlayerGameplayID(const Guid& clientID) {
		auto player = m_players[clientID];
		if (player != nullptr) {
			return player->GetGameplayID();
		}
		return -1;
	}

	void PlayerManagerModule::SetPlayerGameplayID(const Guid& clientID, int groupID) {
		auto player = m_players[clientID];
		if (player != nullptr) {
			player->SetGameplayID(groupID);
		}
	}

void PlayerManagerModule::OnReqPlayerLeave(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len) {
	Guid nPlayerID;
	SquickStruct::ReqLeave xMsg;
	if (!m_pNetModule->ReceivePB(msgID, msg, len, xMsg, nPlayerID)) {
		return;
	}
	dout << "????????????: " << nPlayerID.ToString() << std::endl;

	if (nPlayerID.IsNull()) {
		return;
	}

	// ?????????gameplay
	m_pGameplayManagerModule->GameplayPlayerQuit(nPlayerID);

	// ??????room
	m_pRoomModule->RoomQuit(nPlayerID);

	// ???????????? Gameplay
	
	//m_pKernelModule->SetPropertyInt(nPlayerID, SquickProtocol::IObject::Connection(), 0);
	//m_pKernelModule->DestroyObject(nPlayerID);
	//m_pGameServerNet_ServerModule->RemovePlayerGateInfo(nPlayerID);
}


}