#pragma once

#include "resources.h"
#include "graphics.h"

class CResource_Texture : public CResource
{
	void SetLoaded() { m_State = CResource::STATE_LOADED; }
public:
	CResource_Texture()
	{
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
			return IGraphics::CTextureHandle();
		return ((CResource_Texture *)Get())->m_Handle;
	}
};
