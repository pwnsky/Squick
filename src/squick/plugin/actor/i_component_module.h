

#ifndef SQUICK_INTF_COMPONENT_H
#define SQUICK_INTF_COMPONENT_H

#include <squick/core/platform.h>
#include <squick/core/guid.h>
#include <squick/core/i_module.h>
#include <squick/core/memory_counter.h>

class ActorMessage;
class IComponent;

typedef std::function<void(ActorMessage&)> ACTOR_PROCESS_FUNCTOR;
typedef SQUICK_SHARE_PTR<ACTOR_PROCESS_FUNCTOR> ACTOR_PROCESS_FUNCTOR_PTR;

class ActorMessage
{
public:
	ActorMessage()
	{
		msgID = 0;
		index = 0;
	}

	int msgID;
	uint64_t index;
    Guid id;
	std::string data;
	std::string arg;
protected:
private:
};

class IActor// : MemoryCounter
{
public:
	IActor()
		//: MemoryCounter(GET_CLASS_NAME(IActor), 1)
	{

	}

	virtual ~IActor() {}
	virtual const Guid ID() = 0;
    virtual bool Update() = 0;

	template <typename T>
	SQUICK_SHARE_PTR<T> AddComponent()
	{
		SQUICK_SHARE_PTR<IComponent> component = FindComponent(typeid(T).name());
		if (component)
		{
			return NULL;
		}

		{
			if (!TIsDerived<T, IComponent>::Result)
			{
				return NULL;
			}

			SQUICK_SHARE_PTR<T> component = SQUICK_SHARE_PTR<T>(SQUICK_NEW T());

			assert(NULL != component);

			AddComponent(component);

			return component;
		}

		return nullptr;
	}



	template <typename T>
	SQUICK_SHARE_PTR<T> FindComponent()
	{
		SQUICK_SHARE_PTR<IComponent> component = FindComponent(typeid(T).name());
		if (component)
		{
			SQUICK_SHARE_PTR<T> pT = std::dynamic_pointer_cast<T>(component);

			assert(NULL != pT);

			return pT;
		}

		return nullptr;
	}
	
	template <typename T>
	bool RemoveComponent()
	{
		return RemoveComponent(typeid(T).name());
	}

	virtual bool SendMsg(const ActorMessage& message) = 0;
	virtual bool SendMsg(const int eventID, const std::string& data, const std::string& arg = "") = 0;
	virtual bool BackMsgToMainThread(const ActorMessage& message) = 0;

	virtual bool AddMessageHandler(const int nSubMsgID, ACTOR_PROCESS_FUNCTOR_PTR xBeginFunctor) = 0;

protected:
	virtual bool AddComponent(SQUICK_SHARE_PTR<IComponent> component) = 0;
	virtual bool RemoveComponent(const std::string& componentName) = 0;
	virtual SQUICK_SHARE_PTR<IComponent> FindComponent(const std::string& componentName) = 0;

};

class IComponent// : MemoryCounter
{
private:
    IComponent()
		//: MemoryCounter(GET_CLASS_NAME(IComponent), 1)
    {
    }

public:
    IComponent(const std::string& name)
		//: MemoryCounter(name, 1)
    {
        mbEnable = true;
        mstrName = name;
    }

    virtual ~IComponent() {}

	virtual void SetActor(SQUICK_SHARE_PTR<IActor> self)
	{
		mSelf = self;
	}

	virtual SQUICK_SHARE_PTR<IActor> GetActor()
	{
		return mSelf;
	}

	virtual bool Awake()
	{
		return true;
	}

	virtual bool Start()
	{

		return true;
	}

	virtual bool AfterStart()
	{
		return true;
	}

	virtual bool CheckConfig()
	{
		return true;
	}

	virtual bool ReadyUpdate()
	{
		return true;
	}

	virtual bool Update()
	{
		return true;
	}

	virtual bool BeforeDestory()
	{
		return true;
	}

	virtual bool Destory()
	{
		return true;
	}

	virtual bool Finalize()
	{
		return true;
	}

    virtual bool SetEnable(const bool bEnable)
    {
        mbEnable = bEnable;

        return mbEnable;
    }

    virtual bool Enable()
    {
        return mbEnable;
    }

    virtual const std::string& GetComponentName() const
    {
        return mstrName;
    };

	virtual void ToMemoryCounterString(std::string& info)
	{
		info.append(mSelf->ID().ToString());
		info.append(":");
		info.append(mstrName);
	}

	template<typename BaseType>
	bool AddMsgHandler(const int nSubMessage, BaseType* pBase, int (BaseType::*handler)(ActorMessage&))
	{
		ACTOR_PROCESS_FUNCTOR functor = std::bind(handler, pBase, std::placeholders::_1);
		ACTOR_PROCESS_FUNCTOR_PTR functorPtr(new ACTOR_PROCESS_FUNCTOR(functor));
		
		return mSelf->AddMessageHandler(nSubMessage, functorPtr);
	}

private:
    bool mbEnable;
	SQUICK_SHARE_PTR<IActor> mSelf;
    std::string mstrName;
};

#endif