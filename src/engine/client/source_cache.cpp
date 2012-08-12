
#include "source_cache.h"

void CSource_Cache::GetCacheName(char *pBuffer, int BufferSize, CResource *pResource)
{
	str_format(pBuffer, BufferSize, "%s/%08x_%08x", m_aBaseDirectory,pResource->NameHash(), pResource->ContentHash());
}

bool CSource_Cache::Load(CLoadOrder *pOrder)
{
	// we can't load from the cache if we don't know the hash of the content.
	// we might load the wrong version other wise
	//if(pOrder->m_pResource->ContentHash() == 0)
	//	return false;

	char aFilename[512];
	GetCacheName(aFilename, sizeof(aFilename), pOrder->m_pResource);
	return LoadWholeFile(aFilename, &pOrder->m_pData, &pOrder->m_DataSize) == 0;
}

void CSource_Cache::Feedback(CLoadOrder *pOrder)
{
	char aFilename[512];
	GetCacheName(aFilename, sizeof(aFilename), pOrder->m_pResource);

	IOHANDLE hFile = io_open(aFilename, IOFLAG_WRITE);
	if(hFile)
	{
		io_write(hFile, pOrder->m_pData, pOrder->m_DataSize);
		io_close(hFile);
		dbg_msg("resources", "[%s] saved '%s' to '%s'", Name(), pOrder->m_pResource->Name(), aFilename);
	}
	else
	{
		dbg_msg("resources", "[%s] error opening '%s'", Name(), aFilename);
	}
}	

CSource_Cache::CSource_Cache(const char *pBase)
: CSource_Disk("cache", pBase)
{
}
