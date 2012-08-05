/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_SOUND_H
#define ENGINE_SOUND_H

#include "kernel.h"

class ISound : public IInterface
{
	MACRO_INTERFACE("sound", 0)
protected:
	int m_MixingRate;
public:
	enum
	{
		FLAG_LOOP=1,
		FLAG_POS=2,
		FLAG_ALL=3
	};

	class CSampleHandle
	{
		friend class ISound;
		int m_Id;
	public:
		CSampleHandle()
		: m_Id(-1)
		{}

		operator int() const { return m_Id; }
	};

	int MixingRate() const { return m_MixingRate; }

	virtual bool IsSoundEnabled() = 0;

	virtual CSampleHandle LoadWVFromFile(const char *pFilename) = 0; // TODO: remove this
	virtual CSampleHandle LoadWVFromMem(const void *pData, unsigned DataSize) = 0;

	virtual void SetChannel(int ChannelID, float Volume, float Panning) = 0;
	virtual void SetListenerPos(float x, float y) = 0;

	virtual int PlayAt(int ChannelID, CSampleHandle Sound, int Flags, float x, float y) = 0;
	virtual int Play(int ChannelID, CSampleHandle Sound, int Flags) = 0;
	virtual void Stop(CSampleHandle Sound) = 0;
	virtual void StopAll() = 0;

protected:
	inline CSampleHandle CreateSampleHandle(int Index)
	{
		CSampleHandle Tex;
		Tex.m_Id = Index;
		return Tex;
	}
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
