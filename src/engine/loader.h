#pragma once

#include "kernel.h"

#include <base/tl/ringbuffer.h>

class CJobHandler
{
public:
	class IJob;
	class COrder;
	class CQueue;

	typedef int (*FJob)(CJobHandler *pJobHandler, void *pData);

	class COrder
	{
	public:
		COrder()
		{
			m_pData = 0x0;
			m_pfnProcess = 0x0;
		}

		FJob m_pfnProcess;
		void *m_pData;
	};

	class CQueue
	{
	public:
		CQueue()
		{
			m_WorkerCount = 0;
			m_MaxWorkers = ~(0U); // unlimited by default
		}

		volatile unsigned m_WorkerCount;
		unsigned m_MaxWorkers;
		ringbuffer_swsr<COrder, 1024> m_Orders;
	};

	enum
	{
		NUM_QUEUES = 16,
	};

	CJobHandler() { Init(4); }
	void Init(int ThreadCount);
	void ConfigureQueue(int QueueId, int MaxWorkers); // TODO: not working at the moment

	void *AllocJobData(unsigned DataSize) { return mem_alloc(DataSize, sizeof(void*)); }
	template<typename T> T *AllocJobData() { return (T *)AllocJobData(sizeof(T)); }
	void FreeJobData(void *pPtr) { mem_free(pPtr); }

	void Kick(int QueueId, FJob pfnJob, void *pData);

	unsigned volatile m_WorkDone;
	unsigned volatile m_WorkTurns;

private:
	CQueue m_aQueues[NUM_QUEUES];
	semaphore m_Semaphore;
	lock m_Lock; // TODO: bad performance, this lock can be removed and everything done with waitfree queues

	static void WorkerThread(void *pUser);
};

extern CJobHandler g_JobHandler;

class CResource;
class CResourceHandle;

class CResourceHandle
{
	class CResource *m_pResource;

	inline void Release();
	inline void Assign(CResource *pResource);
public:
	CResourceHandle()
	: m_pResource(0)
	{
	}

	CResourceHandle(CResource *pResource)
	: m_pResource(0)
	{
		Assign(pResource);
	}


	CResourceHandle(const CResourceHandle &rOther)
	: m_pResource(0)
	{
		Assign(rOther.m_pResource);
	}

	~CResourceHandle()
	{
		Release();
	}

	CResource *Get() { return m_pResource; }

	bool IsValid() const { return m_pResource != 0x0; }

	CResourceHandle &operator =(const CResourceHandle &rOther) { Assign(rOther.m_pResource); return *this; }
	CResourceHandle &operator =(CResource *pResource) { Assign(pResource); return *this; }
	CResource *operator->() { assert(m_pResource); return Get(); }
};



/*

this can be rendered with mscgen

msc {
	hscale = "2";
	main, source_start, source_cache, source_disk, source_server, job;

	main->source_start;

	source_start -> source_cache [ label="CSource::m_lInput" ];
	source_cache -> source_disk [  label="CSource::m_lInput" ];
	source_disk -> source_server [ label="CSource::m_lInput" ];
	
	source_server -> source_disk [ label="CSource::m_lFeedback" ];
	source_disk -> source_cache [ label="CSource::m_lFeedback" ];
	source_cache -> source_start [ label="CSource::m_lFeedback" ];

	source_start -> job [ label="Job Data" ];

	job -> main [ label="CResource::m_lInserts" ];
}

	Behaviours:
		* Handlers are called from the loader thread
		* Each source runs on it's own thread
*/
class IResources : public IInterface
{
	MACRO_INTERFACE("resources", 0)
	friend class CResource;
public:
	class IHandler;

	class CResourceId
	{
	public:
		unsigned m_ContentHash;
		unsigned m_NameHash;
		const char *m_pName;
	};

	class CSource
	{
		// a bit ugly
		friend class CResources;
	public:
		class CLoadOrder
		{
		public:
			CResource *m_pResource;
			void *m_pData;
			unsigned m_DataSize;
		};

		CSource(const char *pName);
		virtual ~CSource() {}

		virtual bool Load(CLoadOrder *pOrder) { return false; }
		virtual void Feedback(CLoadOrder *pOrder) { }

		const char *Name() const { return m_pName; }
		IResources *Resources() const { return m_pResources; }
	protected:
		CSource *PrevSource() const { return m_pPrevSource; }
		CSource *NextSource() const { return m_pNextSource; }	
	private:
		const char *m_pName;
		CSource *m_pNextSource;
		CSource *m_pPrevSource;
		IResources *m_pResources;

		ringbuffer_swsr<CLoadOrder, 1024> m_lInput; // previous source write, this source reads
		ringbuffer_swsr<CLoadOrder, 1024> m_lFeedback; // next source write, this source reads
		semaphore m_Semaphore;

		void ForwardOrder(CLoadOrder *pOrder);
		void FeedbackOrder(CLoadOrder *pOrder);
		void Run();
		void Update();

		static void ThreadFunc(void *pThis) { ((CSource *)pThis)->Run(); }
	};

	class IHandler
	{
	public:
		virtual ~IHandler() {}

		// called from the main thread
		virtual CResource *Create(CResourceId Id) = 0;

		// called from job thread
		virtual bool Load(CResource *pResource, void *pData, unsigned DataSize) = 0;

		// called from the main thread during IResources::Update()
		virtual bool Insert(CResource *pResource) = 0;
		virtual bool Destroy(CResource *pResource) = 0;
	};


	virtual ~IResources() {}

	virtual void AssignHandler(const char *pType, IHandler *pHandler) = 0;
	virtual void AddSource(CSource *pSource) = 0;

	virtual void Update() = 0;

	virtual CResourceHandle GetResource(CResourceId Id) = 0;

	CResourceHandle GetResource(const char *pName)
	{
		CResourceId Id;
		Id.m_pName = pName;
		Id.m_NameHash = str_quickhash(pName);
		Id.m_ContentHash = 0;
		return GetResource(Id);
	}

	static IResources *CreateInstance();

private:
	virtual	void Destroy(CResource *pResource) = 0;
};


unsigned hash_crc32(unsigned crc, const void *data, size_t datasize);

class CSource_Disk : public IResources::CSource
{
protected:
	static int LoadWholeFile(const char *pFilename, void **ppData, unsigned *pDataSize);
	char m_aBaseDirectory[512];

	virtual bool Load(CLoadOrder *pOrder);
	CSource_Disk(const char *pName, const char *pBase);
public:
	CSource_Disk(const char *pBase = 0);
	void SetBaseDirectory(const char *pBase);
};

class CResource
{
	friend class IResources;
	friend class CResources;
	friend class CResourceHandle;

	unsigned m_RefCount;

	// only the resource handle should destroy a resource
	void Destroy()
	{
		m_pResources->Destroy(this);
	}

protected:
	// only IResources can destory a resource for good
	virtual ~CResource()
	{
		if(m_Id.m_pName)
			mem_free((void*)m_Id.m_pName);
		m_Id.m_pName = 0x0;
	}

	unsigned m_State;
	IResources::CResourceId m_Id;
	IResources::IHandler *m_pHandler;
	IResources *m_pResources;

	enum
	{
		STATE_ERROR = -1,
		STATE_LOADING = 0,
		STATE_LOADED = 1,
	};

	// only a handler should be able to create a resource
	CResource()
	{
		m_RefCount = 0;
		m_State = STATE_LOADING;
		m_pResources = 0;
		m_pHandler = 0;
		mem_zero(&m_Id, sizeof(m_Id));
	}

public:
	const char *Name() const { return m_Id.m_pName; }
	unsigned NameHash() const { return m_Id.m_NameHash; }
	unsigned ContentHash() const { return m_Id.m_ContentHash; }

	bool IsLoading() const { return m_State == STATE_LOADING; }
	bool IsLoaded() const { return m_State == STATE_LOADED; }
};



void CResourceHandle::Release()
{
	if(m_pResource)
	{
		assert(m_pResource->m_RefCount > 0);
		m_pResource->m_RefCount--;
		if(m_pResource->m_RefCount == 0)
			m_pResource->Destroy();
		m_pResource = 0;
	}
}

void CResourceHandle::Assign(CResource *pResource)
{
	Release();
	if(pResource)
	{
		m_pResource = pResource;
		m_pResource->m_RefCount++;
	}
}


class CResourceIndex
{
	int m_Id;
public:
	explicit CResourceIndex(int Id)
	: m_Id(Id)
	{}

	CResourceIndex()
	: m_Id(-1)
	{}
	
	bool IsValid() const { return m_Id >= 0; }
	int Id() const { return m_Id; }
};
