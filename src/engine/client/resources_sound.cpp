#include <engine/resources_sound.h>

extern "C" { // wavpack
	#include <engine/external/wavpack/wavpack.h>
}

CResource *CResourceHandler_Sound::Create(IResources::CResourceId Id)
{
	return new CResource_Sample();
}

//
__thread const void *CResourceHandler_Sound::ms_pWvBuffer = NULL;
__thread int CResourceHandler_Sound::ms_WvBufferSize = 0;
__thread int CResourceHandler_Sound::ms_WvBufferPos = 0;

int CResourceHandler_Sound::ReadData(void *pBuffer, int BlockSize)
{
	// clamp the block size
	BlockSize = ms_WvBufferSize - ms_WvBufferPos < BlockSize ? ms_WvBufferSize - ms_WvBufferPos : BlockSize;
	mem_copy(pBuffer, (const char *)ms_pWvBuffer + ms_WvBufferPos, BlockSize);
	ms_WvBufferPos += BlockSize;
	return BlockSize;
}

bool CResourceHandler_Sound::Load(CResource *pResource, void *pData, unsigned DataSize)
{
	CResource_Sample *pSample = static_cast<CResource_Sample*>(pResource);
	WavpackContext *pContext;

	//
	ms_pWvBuffer = pData;
	ms_WvBufferSize = DataSize;;
	ms_WvBufferPos = 0;

	char aError[128];
	pContext = WavpackOpenFileInput(ReadData, aError);
	if(!pContext)
		return false;

	int NumSamples = WavpackGetNumSamples(pContext);
	int BitsPerSample = WavpackGetBitsPerSample(pContext);
	unsigned SampleRate = WavpackGetSampleRate(pContext);
	int NumChannels = WavpackGetNumChannels(pContext);

	if(NumChannels > 2 || BitsPerSample != 16)
		return false;

	int *pS32Data = (int *)mem_alloc(4*NumSamples*NumChannels, 1);
	WavpackUnpackSamples(pContext, pS32Data, NumSamples); // TODO: check return value

	int S16Size = 2*NumSamples*NumChannels;
	short *pS16Data = (short *)mem_alloc(S16Size, sizeof(void*));

	// convert down to S16
	{
		int *pSrc = pS32Data;
		short *pDst = pS16Data;
		for (int i = 0; i < NumSamples*NumChannels; i++)
			*pDst++ = (short)*pSrc++;
	}

	mem_free(pS32Data);

	// do rate convert
	const unsigned DstSampleRate = 48000;
	if(SampleRate != DstSampleRate)
	{
		// allocate new data
		int SrcNumFrames = NumSamples/NumChannels;
		int DstNumFrames = (int)((SrcNumFrames/(float)SampleRate)*DstSampleRate);
		short *pNewData = (short *)mem_alloc(DstNumFrames*NumChannels*sizeof(short), sizeof(void*));

		for(int i = 0; i < DstNumFrames; i++)
		{
			// resample TODO: this should be done better, like linear atleast
			float a = i/(float)DstNumFrames;
			int f = (int)(a*SrcNumFrames);
			if(f >= SrcNumFrames)
				f = SrcNumFrames-1;

			// set new data
			if(NumChannels == 1)
				pNewData[i] = pS16Data[f];
			else if(NumChannels == 2)
			{
				pNewData[i*2] = pS16Data[f*2];
				pNewData[i*2+1] = pS16Data[f*2+1];
			}
		}

		// free old data and apply new
		mem_free(pS16Data);
		NumSamples = DstNumFrames*NumChannels;
		pS16Data = pNewData;
		S16Size = NumSamples * sizeof(short);
		SampleRate = DstSampleRate;
	}

	pSample->m_pData = (char*)pS16Data;
	pSample->m_DataSize = S16Size;
	pSample->m_NumChannels = NumChannels;
	pSample->m_SampleRate = SampleRate;
	return true;
}

bool CResourceHandler_Sound::Insert(CResource *pResource)
{
	CResource_Sample *pSample = static_cast<CResource_Sample*>(pResource);
	pSample->m_Handle = m_pSound->LoadRawFromMemTakeOver(pSample->m_pData, pSample->m_DataSize, pSample->m_NumChannels, pSample->m_SampleRate);
	pSample->m_pData = 0x0;
	pSample->m_DataSize = 0;
	return pSample->m_Handle.Id() >= 0;
}


bool CResourceHandler_Sound::Destroy(CResource *pResource)
{

	// make sure that the mixer isn't touching the sample
	//m_pSound->Stop(pSample);
	//pSample->FreeData();

	return true;
}