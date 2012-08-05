#include <engine/resources.h>

class CSource_Cache : public CSource_Disk
{
protected:
	void GetCacheName(char *pBuffer, int BufferSize, CResource *pResource);
	virtual bool Load(CLoadOrder *pOrder);
	virtual void Feedback(CLoadOrder *pOrder);
public:
	CSource_Cache(const char *pBase = 0x0);
};