/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_SOUND_H
#define ENGINE_SOUND_H

#include "kernel.h"
#include <engine/loader.h>

class CResource;

class ISound : public IInterface
{
	MACRO_INTERFACE("sound", 0)
public:
	enum
	{
		FLAG_LOOP=1,
		FLAG_POS=2,
		FLAG_ALL=3
	};

	virtual bool IsSoundEnabled() = 0;

	//virtual int LoadWV(const char *pFilename) = 0;
	virtual CResourceHandle LoadWV(const char *pFilename) = 0;

	virtual void SetChannel(int ChannelID, float Volume, float Panning) = 0;
	virtual void SetListenerPos(float x, float y) = 0;

	virtual int PlayAt(int ChannelID, CResourceHandle Sound, int Flags, float x, float y) = 0;
	virtual int Play(int ChannelID, CResourceHandle Sound, int Flags) = 0;
	virtual void Stop(CResourceHandle Sound) = 0;
	virtual void StopAll() = 0;
};


class IEngineSound : public ISound
{
	MACRO_INTERFACE("enginesound", 0)
public:
	virtual int Init() = 0;
	virtual int Update() = 0;
	virtual int Shutdown() = 0;
};

extern IEngineSound *CreateEngineSound();

#endif
