#include <engine/resources_font.h>

CResource *CResourceHandler_Font::Create(IResources::CResourceId Id)
{
	return new CResource_Font();
}

bool CResourceHandler_Font::Load(CResource *pResource, void *pData, unsigned DataSize)
{
	CResource_Font *pFont = static_cast<CResource_Font*>(pResource);
	
	// just copy the data for now
	pFont->m_pData = new char [DataSize];
	mem_copy(pFont->m_pData, pData, DataSize);
	pFont->m_DataSize = DataSize;
	return false;
}

bool CResourceHandler_Font::Insert(CResource *pResource)
{
	CResource_Font *pFont = static_cast<CResource_Font*>(pResource);
	pFont->m_pFont = m_pTextRender->LoadFont(pFont->m_pData, pFont->m_DataSize);
	return pFont->m_pFont != 0x0;
}


bool CResourceHandler_Font::Destroy(CResource *pResource)
{
	CResource_Font *pFont = static_cast<CResource_Font*>(pResource);
	m_pTextRender->DestroyFont(pFont->m_pFont);
	pFont->m_pFont = 0;

	return true;
}