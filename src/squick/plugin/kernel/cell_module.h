
#ifndef SQUICK_CELL_MODULE_H
#define SQUICK_CELL_MODULE_H

#include <iostream>
#include <assert.h>
#include <map>
#include <squick/core/list.h>
#include <squick/core/data_list.h>
#include <squick/plugin/log/i_log_module.h>
#include <squick/plugin/config/export.h>
#include "i_event_module.h"
#include "i_kernel_module.h"
#include "i_cell_module.h"
#include "i_scene_module.h"

class SceneCellInfo
    : public List<Guid>
{
public:
	SceneCellInfo(const SceneCellInfo& cell)
	{
		msceneID = cell.msceneID;
		mgroupID = cell.mgroupID;
		mCellID = cell.mCellID;

		for (int i = ECELL_TOP; i < ECELL_DIRECTION_MAXCOUNT; i++)
		{
			mAroundCell[i] = NULL;
		}
	}

    SceneCellInfo(const int& sceneID, const int& groupID, const Guid& cellID)
    {
		msceneID = sceneID;
		mgroupID = groupID;
		mCellID = cellID;

        for (int i = ECELL_TOP; i < ECELL_DIRECTION_MAXCOUNT; i++)
        {
            mAroundCell[i] = NULL;
        }
    }

    virtual ~SceneCellInfo()
    {
        // TODO
    }

    void Start(SQUICK_SHARE_PTR<SceneCellInfo>* pGridArray)
    {
        for (int i = ECELL_TOP; i < ECELL_DIRECTION_MAXCOUNT; i++)
        {
            mAroundCell[i] = pGridArray[i];
        }
    }

    void Finale()
    {
    }

	SQUICK_SHARE_PTR<SceneCellInfo> GetConnectCell(ECELL_DIRECTION eDirection)
    {
        return mAroundCell[eDirection];
    }

	const int GetSceneID()
	{
		return msceneID;
	}

	const int GetGroupID()
	{
		return mgroupID;
	}
	const Guid GetID()
	{
		return mCellID;
	}
	const bool Exist(const Guid& id)
	{
		return this->Find(id);
	}

protected:
private:
    SQUICK_SHARE_PTR<SceneCellInfo> mAroundCell[ECELL_DIRECTION_MAXCOUNT];
	Guid mCellID;
	int msceneID;
	int mgroupID;
};

class CellModule
    : public ICellModule
{
public:
	CellModule(IPluginManager* p);

    virtual ~CellModule();

	virtual bool Start();
	virtual bool AfterStart();
	virtual bool BeforeDestory();
	virtual bool Destory();
	virtual bool Update();

	virtual const bool CreateGroupCell(const int& sceneID, const int& groupID);
	virtual const bool DestroyGroupCell(const int& sceneID, const int& groupID);

    // the event that a object are moving
    virtual const Guid OnObjectMove(const Guid& self, const int& sceneID, const int& groupID, const Guid& fromCell, const Guid& toCell);

    // the event that a object has entried
    virtual const Guid OnObjectEntry(const Guid& self, const int& sceneID, const int& groupID, const Guid& toCell);

    // the event that a object has leaved
    virtual const Guid OnObjectLeave(const Guid& self, const int& sceneID, const int& groupID, const Guid& fromCell);

	virtual bool GetCellObjectList(const int sceneID, const int groupID, const Vector3& pos, DataList& list, ECELL_AROUND around = ECELL_AROUND_9);
	virtual bool GetCellObjectList(const int sceneID, const int groupID, const Vector3& pos, DataList& list, const Guid& noSelf, ECELL_AROUND around = ECELL_AROUND_9);
	virtual bool GetCellObjectList(const int sceneID, const int groupID, const Vector3& pos, DataList& list, const bool bPlayer, ECELL_AROUND around = ECELL_AROUND_9);
	virtual bool GetCellObjectList(const int sceneID, const int groupID, const Vector3& pos, DataList& list, const bool bPlayer, const Guid& noSelf, ECELL_AROUND around = ECELL_AROUND_9);
	virtual bool GetCellObjectList(const int sceneID, const int groupID, const Vector3& pos, const std::string& className, DataList& list, ECELL_AROUND around = ECELL_AROUND_9);
	virtual bool GetCellObjectList(const int sceneID, const int groupID, const Vector3& pos, const std::string& className, DataList& list, const Guid& noSelf, ECELL_AROUND around = ECELL_AROUND_9);
    
	//////////////////////////////////////////////////////////////////////////
    // computer a id of this grid by position
	virtual const Guid ComputeCellID(const int nX, const int nY, const int nZ);
	virtual const Guid ComputeCellID(const Vector3& vec);
	//////////////////////////////////////////////////////////////////////////
	// computer a id of this grid by position
	virtual const Guid ComputeCellID(const Guid& selfGrid, ECELL_DIRECTION eDirection);

    // get the step lenth each two grid
    virtual const Guid GetStepLenth(const Guid& selfGrid, const Guid& otherGrid);

    // get some grids that around this grid(not include self)
    virtual const int GetAroundCell(const int& sceneID, const int& groupID, const Guid& selfGrid, SQUICK_SHARE_PTR<SceneCellInfo>* gridList,
                                    ECELL_AROUND around = ECELL_AROUND_9);


    // get some objects that around this grid(not include self)
    virtual const int GetAroundObject(const int& sceneID, const int& groupID, const Guid& selfGrid, DataList& objectList,
                                      ECELL_AROUND around = ECELL_AROUND_9);


    // get a grid who connected it by direction
    virtual SQUICK_SHARE_PTR<SceneCellInfo> GetConnectCell(const int& sceneID, const int& groupID, const Guid& selfGrid, ECELL_DIRECTION eDirection);

    // get the pointer of this grid
    virtual SQUICK_SHARE_PTR<SceneCellInfo> GetCellInfo(const int& sceneID, const int& groupID, const Guid& selfGrid);

protected:

	// get some grids that around this grid(not include self)
	virtual const int GetAroundCell(SQUICK_SHARE_PTR<SceneCellInfo> pGridInfo, SQUICK_SHARE_PTR<SceneCellInfo>* gridList,
		ECELL_AROUND around = ECELL_AROUND_9);

	// get some objects that around this grid(not include self)
	virtual const int GetAroundObject(SQUICK_SHARE_PTR<SceneCellInfo> pGridInfo, DataList& objectList,
		ECELL_AROUND around = ECELL_AROUND_9);


	virtual int AddMoveEventCallBack(CELL_MOVE_EVENT_FUNCTOR_PTR functorPtr);
	virtual int AddMoveInEventCallBack(CELL_MOVE_EVENT_FUNCTOR_PTR functorPtr);
	virtual int AddMoveOutEventCallBack(CELL_MOVE_EVENT_FUNCTOR_PTR functorPtr);

	int OnObjectEvent(const Guid& self, const std::string& classNames, const CLASS_OBJECT_EVENT classEvent, const DataList& var);
	int OnPositionEvent(const Guid & self, const std::string & propertyName, const SquickData & oldVar, const SquickData & newVar, const INT64 reason);

private:

	bool OnMoveEvent(const Guid& self, const int& sceneID, const int& groupID, const Guid& fromCell, const Guid& toCell);
	bool OnMoveInEvent(const Guid& self, const int& sceneID, const int& groupID, const Guid& toCell);
	bool OnMoveOutEvent(const Guid& self, const int& sceneID, const int& groupID, const Guid& fromCell);
	
	int BeforeLeaveSceneGroup(const Guid& self, const int sceneID, const int groupID, const int type, const DataList& argList);
	int AfterEnterSceneGroup(const Guid& self, const int sceneID, const int groupID, const int type, const DataList& argList);

private:
	const static int nCellWidth = 10;

	//cell_id -> cell_data
	typedef std::map<Guid, SQUICK_SHARE_PTR<SceneCellInfo>> TMAP_CELL_INFO;

	//groupd_id->cell info
	typedef std::map<int, TMAP_CELL_INFO> TMAP_GROUP_INFO;

	//scene_id->group info
	typedef std::map<int, TMAP_GROUP_INFO> TMAP_SCENE_INFO;
	TMAP_SCENE_INFO mtCellInfoMap;


	std::vector<CELL_MOVE_EVENT_FUNCTOR_PTR> mMoveEventHandler;
	std::vector<CELL_MOVE_EVENT_FUNCTOR_PTR> mMoveInEventHandler;
	std::vector<CELL_MOVE_EVENT_FUNCTOR_PTR> mMoveOutEventHandler;
private:
	IKernelModule* m_pKernelModule;
	IClassModule* m_pClassModule;
	ILogModule* m_pLogModule;
	IElementModule* m_pElementModule;
	IEventModule* m_pEventModule;
};

#endif
