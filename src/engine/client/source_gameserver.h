#include <engine/resources.h>

class CSource_GameServer : public IResources::CSource
{
public:
	class CChunk
	{
	public:
		unsigned m_DataSize;
		char m_aData[];

		static CChunk *Create(const void *pData, unsigned DataSize);
	};
protected:
	ringbuffer_swsr<CChunk*, 64> m_lpInputChunks; // main thread writes, source thread reads
	ringbuffer_swsr<CChunk*, 64> m_lpOutputChunks; // source thread writes, main thread reads

	volatile int m_Active; // main thread writes, source thread reads
	semaphore m_Activity;

	unsigned m_DataOffset;
	bool m_Done;

	CLoadOrder *m_pOrder;

	void SendMsg(class CMsgPacker *pMsg);
	void SendNextFetch();

	bool ProcessChunk(CChunk *pChunk);
	virtual bool Load(CLoadOrder *pOrder);

public:
	CSource_GameServer();

	void SetActive(bool Active);
	void QueueChunk(const void *pData, unsigned DataSize);
	CChunk *PopOutputChunk();
};
