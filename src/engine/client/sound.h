/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_CLIENT_SOUND_H
#define ENGINE_CLIENT_SOUND_H

#include <engine/sound.h>

class CSound : public IEngineSound
{
	int m_SoundEnabled;

	// TODO: Refactor: clean this mess up
	static const void *ms_pWvBuffer;
	static int ms_WvBufferSize;
	static int ms_WvBufferPos;
	static int ReadData(void *pBuffer, int Size);

public:
	class IEngineGraphics *m_pGraphics; // TODO: remove this dependency
	class IStorage *m_pStorage; // TODO: remove this dependency

	virtual int Init();

	int Update();
	int Shutdown();
	int AllocID();

	void RateConvert(int SampleID);

	virtual bool IsSoundEnabled() { return m_SoundEnabled != 0; }

	virtual CSampleHandle LoadWVFromFile(const char *pFilename);
	virtual CSampleHandle LoadWVFromMem(const void *pData, unsigned DataSize);
	virtual CSampleHandle LoadRawFromMem(const void *pData, unsigned DataSize, int Channels, int SampleRate);
	virtual CSampleHandle LoadRawFromMemTakeOver(const void *pData, unsigned DataSize, int Channels, int SampleRate);

	virtual void SetListenerPos(float x, float y);
	virtual void SetChannel(int ChannelID, float Vol, float Pan);

	int Play(int ChannelID, CSampleHandle SampleID, int Flags, float x, float y);
	virtual int PlayAt(int ChannelID, CSampleHandle SampleID, int Flags, float x, float y);
	virtual int Play(int ChannelID, CSampleHandle SampleID, int Flags);
	virtual void Stop(CSampleHandle SampleID);
	virtual void StopAll();
};

#endif
