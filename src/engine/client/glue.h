#pragma once

#include <engine/loader.h>
#include <engine/graphics.h>
#include <engine/sound.h>

class CResource_Sample : public CResource
{
	friend class CResourceHandler_Sound;
	char *m_pData;
	int m_DataSize;

	ISound::CSampleHandle m_Handle;
public:
	CResource_Sample()
	: m_Handle(-1)
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
			return ISound::CSampleHandle(-1);
		return ((CResource_Sample *)Get())->GetId();
	}
};

class CResource_Texture : public CResource
{
	void SetLoaded() { m_State = CResource::STATE_LOADED; }
public:
	CResource_Texture()
	{
		m_Handle = IGraphics::CTextureHandle(-1);
		m_MemSize = 0;
		m_Flags = 0;
		mem_zero(&m_ImageInfo, sizeof(m_ImageInfo));
	}

	IGraphics::CTextureHandle m_Handle;
	int m_MemSize;
	int m_Flags;

	// used for loading the texture
	// TODO: should perhaps just be stored at load time
	CImageInfo m_ImageInfo;
};

class CResourceHandler_Texture : public IResources::IHandler
{
public:
	IGraphics *m_pGraphics;
	static unsigned int PngReadFunc(void *pOutput, unsigned long size, unsigned long numel, void *pUserPtr);
	virtual CResource *Create(IResources::CResourceId Id);
	virtual bool Load(CResource *pResource, void *pData, unsigned DataSize);
	virtual bool Insert(CResource *pResource);
	virtual bool Destroy(CResource *pResource);
};

class CResourceHandleTexture : public CResourceHandle
{
public:
	CResourceHandleTexture() {}
	CResourceHandleTexture(const CResourceHandle &other)
	: CResourceHandle(other)
	{}
	
	operator IGraphics::CTextureHandle() const
	{
		if(!Get())
			return IGraphics::CTextureHandle(-1);
		return ((CResource_Texture *)Get())->m_Handle;
	}
};

/*
// TODO: ugly way of solving this but it has todo for now.
inline IGraphics::CTextureHandle operator+(const CResourceHandle &rHandle)
{
	if(!rHandle.Get())
		return IGraphics::CTextureHandle(-1);
	return ((CResource_Texture *)rHandle.Get())->m_TexId;
}*/



