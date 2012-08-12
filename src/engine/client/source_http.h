#include <engine/resources.h>

class CSource_Http : public IResources::CSource
{
protected:
	struct SHttpHeaderInfo
	{
		int m_ResponseCode;
		int m_ContentLength;
	};

	enum
	{
		RESPONSECODE_OK = 200,
	};

	SHttpHeaderInfo ParseHeader(const char *pHeader, int HeaderLength);

	virtual bool Load(CLoadOrder *pOrder);

public:
	CSource_Http();
};
