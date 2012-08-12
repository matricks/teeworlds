/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_MAPIMAGES_H
#define GAME_CLIENT_COMPONENTS_MAPIMAGES_H
#include <engine/resources_texture.h>
#include <game/client/component.h>

class CMapImages : public CComponent
{
	CResourceHandleTexture m_aTextures[64];
	CResourceHandleTexture m_aMenuTextures[64];
	int m_Count;
	int m_MenuCount;
public:
	CMapImages();

	CResourceHandleTexture Get(int Index) const;
	int Num() const;

	virtual void OnMapLoad();
	void OnMenuMapLoad(class IMap *pMap);
};

#endif
