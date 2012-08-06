#pragma once

#include "resources.h"
#include "graphics.h"

class CResource_Texture : public CResource
{
	void SetLoaded() { m_State = CResource::STATE_LOADED; }
public:
	CResource_Texture()
	{
		mem_zero(&m_ImageInfo, sizeof(m_ImageInfo));
	}

	IGraphics::CTextureHandle m_Handle;

	// used for loading the texture
	CImageInfo m_ImageInfo; // TODO: should perhaps just be stored at load time
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
