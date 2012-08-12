#pragma once

#include <engine/resources.h>
#include <engine/sound.h>

class CResource_Sample : public CResource
{
	friend class CResourceHandler_Sound;
	
	// used for loading
	char *m_pData;
	int m_DataSize;
	int m_NumChannels;
	int m_SampleRate;

	ISound::CSampleHandle m_Handle;
public:
	CResource_Sample()
	{
		m_pData = 0;
		m_DataSize = 0;
	}

	virtual ~CResource_Sample()
	{
	}

	ISound::CSampleHandle GetId() { return m_Handle; }
};

class CResourceHandler_Sound : public IResources::IHandler
{
	static __thread const void *ms_pWvBuffer;
	static __thread int ms_WvBufferSize;
	static __thread int ms_WvBufferPos;
	static __thread int ReadData(void *pBuffer, int Size);
public:
	ISound *m_pSound;
	virtual CResource *Create(IResources::CResourceId Id);
	virtual bool Load(CResource *pResource, void *pData, unsigned DataSize);
	virtual bool Insert(CResource *pResource);
	virtual bool Destroy(CResource *pResource);
};


class CResourceHandleSound : public CResourceHandle
{
public:
	CResourceHandleSound() {}
	CResourceHandleSound(const CResourceHandle &other)
	: CResourceHandle(other)
	{}

	operator ISound::CSampleHandle() const
	{
		if(!Get())
			return ISound::CSampleHandle();
		return ((CResource_Sample *)Get())->GetId();
	}
};

