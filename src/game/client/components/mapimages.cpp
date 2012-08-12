/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/graphics.h>
#include <engine/map.h>
#include <engine/storage.h>
#include <game/client/component.h>
#include <game/mapitems.h>

#include "mapimages.h"

CMapImages::CMapImages()
{
	m_Count = 0;
	m_MenuCount = 0;
}

void CMapImages::OnMapLoad()
{
	IMap *pMap = Kernel()->RequestInterface<IMap>();

	// unload all textures
	for(int i = 0; i < m_Count; i++)
		m_aTextures[i].Release();
	m_Count = 0;

	int Start;
	pMap->GetType(MAPITEMTYPE_IMAGE, &Start, &m_Count);

	// load new textures
	for(int i = 0; i < m_Count; i++)
	{
		CMapItemImage *pImg = (CMapItemImage *)pMap->GetItem(Start+i, 0, 0);
		if(pImg->m_External || (pImg->m_Version > 1 && pImg->m_Format != CImageInfo::FORMAT_RGB && pImg->m_Format != CImageInfo::FORMAT_RGBA))
		{
			char aBuf[256];
			char *pName = (char *)pMap->GetData(pImg->m_ImageName);
			str_format(aBuf, sizeof(aBuf), "mapres/%s.png", pName);
			m_aTextures[i] = Resources()->GetResource(aBuf);
		}
		else
		{
			// NOTE: images baked into the map is not supported anymore
		}
	}
}

void CMapImages::OnMenuMapLoad(IMap *pMap)
{
	// unload all textures
	for(int i = 0; i < m_MenuCount; i++)
		m_aMenuTextures[i].Release();
	m_MenuCount = 0;

	int Start;
	pMap->GetType(MAPITEMTYPE_IMAGE, &Start, &m_MenuCount);

	// load new textures
	for(int i = 0; i < m_MenuCount; i++)
	{
		CMapItemImage *pImg = (CMapItemImage *)pMap->GetItem(Start+i, 0, 0);
		if(pImg->m_External || (pImg->m_Version > 1 && pImg->m_Format != CImageInfo::FORMAT_RGB && pImg->m_Format != CImageInfo::FORMAT_RGBA))
		{
			char aBuf[256];
			char *pName = (char *)pMap->GetData(pImg->m_ImageName);
			str_format(aBuf, sizeof(aBuf), "mapres/%s.png", pName);
			m_aMenuTextures[i] = Resources()->GetResource(aBuf);
		}
		else
		{
			// NOTE: images baked into the map is not supported anymore
		}
	}
}

CResourceHandleTexture CMapImages::Get(int Index) const
{
	if(Client()->State() == IClient::STATE_ONLINE || Client()->State() == IClient::STATE_DEMOPLAYBACK)
		return m_aTextures[Index];
	return m_aMenuTextures[Index];
}

int CMapImages::Num() const
{
	if(Client()->State() == IClient::STATE_ONLINE || Client()->State() == IClient::STATE_DEMOPLAYBACK)
		return m_Count;
	return m_MenuCount;
}
