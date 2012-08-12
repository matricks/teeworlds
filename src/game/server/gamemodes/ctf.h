/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_GAMEMODES_CTF_H
#define GAME_SERVER_GAMEMODES_CTF_H
#include <game/server/gamecontroller.h>
#include <game/server/entity.h>

class CGameControllerCTF : public IGameController
{
	// balancing
	virtual bool CanBeMovedOnBalance(int ClientID);

	// game
	class CFlag *m_apFlags[2];

	CResourceIndex m_Sound_FlagDrop;
	CResourceIndex m_Sound_FlagReturn;
	CResourceIndex m_Sound_FlagCapture;
	CResourceIndex m_Sound_FlagGrabEn;
	CResourceIndex m_Sound_FlagGrabPl;

	virtual void DoWincheckMatch();

public:
	CGameControllerCTF(class CGameContext *pGameServer);
	
	// event
	virtual int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon);
	virtual bool OnEntity(int Index, vec2 Pos);

	// general
	virtual void Snap(int SnappingClient);
	virtual void Tick();
};

#endif

