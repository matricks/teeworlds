
#include "glue.h"

extern "C" { // wavpack
	#include <engine/external/wavpack/wavpack.h>
}


CResource *CResourceHandler_Sound::Create(IResources::CResourceId Id)
{
	return new CResource_Sample();
}

// Ugly TLS solution
/*
__thread char *gt_pWVData;
__thread int gt_WVDataSize;
static int ThreadReadData(void *pBuffer, int ChunkSize)
{
	if(ChunkSize > gt_WVDataSize)
		ChunkSize = gt_WVDataSize;
	mem_copy(pBuffer, gt_pWVData, ChunkSize);
	gt_pWVData += ChunkSize;
	gt_WVDataSize -= ChunkSize;
	return ChunkSize;
}*/

bool CResourceHandler_Sound::Load(CResource *pResource, void *pData, unsigned DataSize)
{
	CResource_Sample *pSample = static_cast<CResource_Sample*>(pResource);
	
	// just copy the data for now
	pSample->m_pData = new char [DataSize];
	mem_copy(pSample->m_pData, pData, DataSize);
	pSample->m_DataSize = DataSize;
	return false;

	/*
	char aError[100];
	gt_pWVData = (char*)pData;
	gt_WVDataSize = DataSize;
	WavpackContext *pContext = WavpackOpenFileInput(ThreadReadData, aError);
	if (pContext)
	{
		int NumSamples = WavpackGetNumSamples(pContext);
		int BitsPerSample = WavpackGetBitsPerSample(pContext);
		unsigned int SampleRate = WavpackGetSampleRate(pContext);
		int NumChannels = WavpackGetNumChannels(pContext);

		if(NumChannels > 2)
		{
			dbg_msg("sound/wv", "file is not mono or stereo. filename='%s'", pResource->Name());
			return -1;
		}


		if(BitsPerSample != 16)
		{
			dbg_msg("sound/wv", "bps is %d, not 16, filname='%s'", BitsPerSample, pResource->Name());
			return -1;
		}

		short *pFinalData = (short *)mem_alloc(2*NumSamples*NumChannels, 1);

		{
			int *pTmpData = (int *)mem_alloc(4*NumSamples*NumChannels, 1);
			WavpackUnpackSamples(pContext, pTmpData, NumSamples); // TODO: check return value

			// convert int32 to int16
			{
				int *pSrc = pTmpData;
				short *pDst = pFinalData;
				for(int i = 0; i < NumSamples*NumChannels; i++)
					*pDst++ = (short)*pSrc++;
			}

			mem_free(pTmpData);
		}

		// do rate convert
		{
			int NewNumFrames = 0;
			short *pNewData = 0;

			// allocate new data
			NewNumFrames = (int)((NumSamples / (float)SampleRate)*m_MixingRate);
			pNewData = (short *)mem_alloc(NewNumFrames*NumChannels*sizeof(short), sizeof(void*));

			for(int i = 0; i < NewNumFrames; i++)
			{
				// resample TODO: this should be done better, like linear atleast
				float a = i/(float)NewNumFrames;
				int f = (int)(a*NumSamples);
				if(f >= NumSamples)
					f = NumSamples-1;

				// set new data
				if(NumChannels == 1)
					pNewData[i] = pFinalData[f];
				else if(NumChannels == 2)
				{
					pNewData[i*2] = pFinalData[f*2];
					pNewData[i*2+1] = pFinalData[f*2+1];
				}
			}

			// free old data and apply new
			mem_free(pFinalData);
			pFinalData = pNewData;
			NumSamples = NewNumFrames;
		}


		// insert it directly, we don't need to wait for anything
		pSample->m_Channels = NumChannels;
		pSample->m_Rate = SampleRate;
		pSample->m_LoopStart = -1;
		pSample->m_LoopEnd = -1;
		pSample->m_PausedAt = 0;
		pSample->m_pData = pFinalData;
		sync_barrier(); // make sure that all parameters are written before we say how large it is
		pSample->m_NumFrames = NumSamples;
	}
	else
	{
		dbg_msg("sound/wv", "failed to open %s: %s", pResource->Name(), aError);
	}

	//RateConvert(SampleID);
	return 0;
	*/
}

bool CResourceHandler_Sound::Insert(CResource *pResource)
{
	CResource_Sample *pSample = static_cast<CResource_Sample*>(pResource);
	pSample->m_Handle = m_pSound->LoadWVFromMem(pSample->m_pData, pSample->m_DataSize);
	delete [] pSample->m_pData;
	return pSample->m_Handle >= 0;
}


bool CResourceHandler_Sound::Destroy(CResource *pResource)
{

	// make sure that the mixer isn't touching the sample
	//m_pSound->Stop(pSample);
	//pSample->FreeData();

	return true;
}

#include <engine/external/pnglite/pnglite.h>

unsigned int CResourceHandler_Texture::PngReadFunc(void *pOutput, unsigned long size, unsigned long numel, void *pUserPtr)
{
	unsigned char **pData = reinterpret_cast<unsigned char**>(pUserPtr);
	unsigned long TotalSize = size*numel;
	if(pOutput)
		mem_copy(pOutput, *pData, TotalSize);
	(*pData) += TotalSize;
	return TotalSize;
}

// called from the main thread
CResource *CResourceHandler_Texture::Create(IResources::CResourceId Id)
{
	return new CResource_Texture;
}

// called from job thread
bool CResourceHandler_Texture::Load(CResource *pResource, void *pData, unsigned DataSize)
{
	CResource_Texture *pTexture = static_cast<CResource_Texture*>(pResource);

	png_t Png;
	
	int Error = png_open(&Png, PngReadFunc, &pData); // ignore_convention
	if(Error != PNG_NO_ERROR)
	{
		dbg_msg("graphics", "failed to open file. name='%s'", pResource->Name());
		return -1;// 0;
	}

	if(Png.depth != 8 || (Png.color_type != PNG_TRUECOLOR && Png.color_type != PNG_TRUECOLOR_ALPHA)) // ignore_convention
	{
		dbg_msg("graphics", "invalid format. filename='%s'", pResource->Name());
		return -2;// 0;
	}

	unsigned char *pBuffer = (unsigned char *)mem_alloc(Png.width * Png.height * Png.bpp, 1); // ignore_convention
	png_get_data(&Png, pBuffer); // ignore_convention

	pTexture->m_ImageInfo.m_Width = Png.width; // ignore_convention
	pTexture->m_ImageInfo.m_Height = Png.height; // ignore_convention
	if(Png.color_type == PNG_TRUECOLOR) // ignore_convention
		pTexture->m_ImageInfo.m_Format = CImageInfo::FORMAT_RGB;
	else if(Png.color_type == PNG_TRUECOLOR_ALPHA) // ignore_convention
		pTexture->m_ImageInfo.m_Format = CImageInfo::FORMAT_RGBA;
	pTexture->m_ImageInfo.m_pData = pBuffer;
	return 0;
}

// called from the main thread
bool CResourceHandler_Texture::Insert(CResource *pResource)
{
	CResource_Texture *pTexture = static_cast<CResource_Texture*>(pResource);
	CImageInfo *pInfo = &pTexture->m_ImageInfo;
	pTexture->m_Handle = m_pGraphics->LoadTextureRaw(pInfo->m_Width, pInfo->m_Height, pInfo->m_Format, pInfo->m_pData, pInfo->m_Format, 0);
	// free the texture data
	mem_free(pInfo->m_pData);
	pInfo->m_pData = 0;

	return true;
}

bool CResourceHandler_Texture::Destroy(CResource *pResource)
{
	CResource_Texture *pTexture = static_cast<CResource_Texture*>(pResource);
	m_pGraphics->UnloadTexture(pTexture->m_Handle);
	pTexture->m_Handle = IGraphics::CTextureHandle();
	return true;
}




