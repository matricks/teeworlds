/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_CLIENT_SOUND_H
#define ENGINE_CLIENT_SOUND_H

#include <base/tl/base.h>
#include <engine/sound.h>
#include <engine/loader.h>



class CSound : public IEngineSound
{
	int m_SoundEnabled;
public:
	class CResource_Sample : public CResource
	{
	public:
		CResource_Sample()
		{
			m_pData = 0x0;
			m_NumFrames = 0;
			m_Rate = 0;
			m_Channels = 0;
			m_LoopStart = 0;
			m_LoopEnd = 0;
			m_PausedAt = 0;
		}

		virtual ~CResource_Sample()
		{
			assert(m_pData == 0);
		}

		void FreeData()
		{
			if(m_pData)
				mem_free(m_pData);
			m_pData = 0x0;
		}

		short *m_pData;
		int m_NumFrames;
		int m_Rate;
		int m_Channels;
		int m_LoopStart;
		int m_LoopEnd;
		int m_PausedAt;
	};

	class CResourceHandler : public IResources::IHandler
	{
	public:
		CSound *m_pSound;
		virtual CResource *Create(IResources::CResourceId Id);
		virtual bool Load(CResource *pResource, void *pData, unsigned DataSize);
		virtual bool Insert(CResource *pResource);
		virtual bool Destroy(CResource *pResource);
	};

	CResourceHandler m_ResourceHandler;
		
	IResources *m_pResources;
	IEngineGraphics *m_pGraphics;
	IStorage *m_pStorage;

	virtual int Init();

	int Update();
	int Shutdown();

	virtual bool IsSoundEnabled() { return m_SoundEnabled != 0; }

	virtual CResourceHandle LoadWV(const char *pFilename);

	virtual void SetListenerPos(float x, float y);
	virtual void SetChannel(int ChannelID, float Vol, float Pan);

	int Play(int ChannelID, CResourceHandle Sound, int Flags, float x, float y);
	virtual int PlayAt(int ChannelID, CResourceHandle Sound, int Flags, float x, float y);
	virtual int Play(int ChannelID, CResourceHandle Sound, int Flags);
	virtual void Stop(CResourceHandle Sound);
	virtual void StopAll();
};

#endif
