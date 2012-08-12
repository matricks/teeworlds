#pragma once

#include "resources.h"
#include "textrender.h"

class CResource_Font : public CResource
{
	friend class CResourceHandler_Font;
	friend class CResourceHandleFont;

	char *m_pData;
	int m_DataSize;

	CFont *m_pFont;
public:
	CResource_Font()
	{
		m_pFont = 0;

		m_pData = 0;
		m_DataSize = 0;
	}
};

class CResourceHandler_Font : public IResources::IHandler
{
public:
	ITextRender *m_pTextRender;
	virtual CResource *Create(IResources::CResourceId Id);
	virtual bool Load(CResource *pResource, void *pData, unsigned DataSize);
	virtual bool Insert(CResource *pResource);
	virtual bool Destroy(CResource *pResource);
};

class CResourceHandleFont : public CResourceHandle
{
public:
	CResourceHandleFont() {}
	CResourceHandleFont(const CResourceHandle &other)
	: CResourceHandle(other)
	{}
	
	operator CFont *() const
	{
		if(!Get())
			return 0x0;
		return ((CResource_Font *)Get())->m_pFont;
	}
};
