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
		if(pImg->m_External)
		{
			char aBuf[256];
			char *pName = (char *)pMap->GetData(pImg->m_ImageName);
			str_format(aBuf, sizeof(aBuf), "mapres/%s.png", pName);
			m_aTextures[i] = Resources()->GetResource(aBuf);
		}
		else
		{
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "mapimage%d", i);

			//void *pData = pMap->GetData(pImg->m_ImageData);
			// TODO: what do we do here?

			//m_aTextures[i] = Graphics()->LoadTextureRaw(aBuf, pImg->m_Width, pImg->m_Height, CImageInfo::FORMAT_RGBA, pData, CImageInfo::FORMAT_RGBA, 0);
			//m_aTextures[i] = Graphics()->LoadTextureRaw(aBuf, pImg->m_Width, pImg->m_Height, CImageInfo::FORMAT_RGBA, pData, CImageInfo::FORMAT_RGBA, 0);
			//pMap->UnloadData(pImg->m_ImageData);
		}
	}
}

