
#include <engine/message.h>

#include "source_http.h"

CSource_Http::CSource_Http()
: CSource("http")
{
}

CSource_Http::SHttpHeaderInfo CSource_Http::ParseHeader(const char *pHeader, int HeaderLength)
{
	SHttpHeaderInfo Info = {0};

	// check response line
	// HTTP/1.0 200 OK\r\n
	if(str_comp_num(pHeader, "HTTP/1.1", 8) != 0)
		return Info;

	// dig out the response code
	Info.m_ResponseCode = str_toint(pHeader + sizeof("HTTP/1.1"));

	// process header lines
	const char *pLine = pHeader;
	const char *pHeaderEnd = &pHeader[HeaderLength];

	const char ContentLengthStr[] = "Content-Length:";

	// scan all the lines
	while(pLine < pHeaderEnd)
	{
		const char *pLineEnd = pLine;
		while(*pLineEnd != '\r')
			pLineEnd++;

		int LineLength = pLineEnd - pLine;
		if(LineLength > (int)sizeof(ContentLengthStr) && str_comp_num(pLine, ContentLengthStr, sizeof(ContentLengthStr)-1) == 0)
		{
			// found content length header line
			Info.m_ContentLength = str_toint(pLine+sizeof(ContentLengthStr));

		}

		// next line
		pLine = pLineEnd+2; // skip \r\n
	}

	return Info;
}


bool CSource_Http::Load(CLoadOrder *pOrder)
{
	const char *pHostname = "matricks.se";
	int Port = 80;
	const char *pBasePath = "/twdata/";

	// fetch address
	NETADDR HostAddr;
	if(net_host_lookup(pHostname, &HostAddr, NETTYPE_ALL) != 0)
	{
		dbg_msg("http", "host lookup failed");
		return false;
	}

	// create socket
	NETADDR BindAddr = {0};
	BindAddr.type = HostAddr.type;
	NETSOCKET Socket = net_tcp_create(BindAddr);

	if(!Socket.type)
	{
		dbg_msg("http", "failed to open socket");
		return false;
	}

	// connect
	HostAddr.port = Port;
	if(net_tcp_connect(Socket, &HostAddr) != 0)
	{
		dbg_msg("http", "failed to connect");
		return false;
	}

	// send request
	char aBuf[1024];
	str_format(aBuf, sizeof(aBuf),
		"GET %s/%s HTTP/1.1\r\n"
		"Host: %s:%d\r\n"
		"Connection: close\r\n"
		"\r\n",
		pBasePath, pOrder->m_pResource->Name(),
		pHostname, Port);

	net_tcp_send(Socket, aBuf, str_length(aBuf));

	// header 
	SHttpHeaderInfo HeaderInfo = {0};

	//recive header (max 4k)
	char aHeader[4*1024];
	int HeaderEnd = -1;
	int HeaderDataSize = 0;

	for(;;)
	{
		// fetch more data
		int ChunkSize = sizeof(aHeader) - HeaderDataSize;
		int NumBytes = net_tcp_recv(Socket, &aHeader[HeaderDataSize], ChunkSize);
		if(NumBytes < 0 || NumBytes > ChunkSize)
			break;
		HeaderDataSize += NumBytes;

		// check for header end
		for(int i = 0; i < HeaderDataSize-3; i++)
		{
			if(aHeader[i] == '\r' && aHeader[i+1] == '\n' && aHeader[i+2] == '\r' && aHeader[i+3] == '\n')
			{
				HeaderEnd = i;
				HeaderInfo = ParseHeader(aHeader, HeaderEnd);
				HeaderEnd += 4; // skip \r\n\r\n

				break;
			}
		}

		// okey, we found the end of the header
		if(HeaderEnd >= 0)
		{
			// TODO: dig out response code
			// TODO: dig out content length
			break;
		}
	}

	//dbg_msg("http", "header code=%d content=%d", HeaderInfo.m_ResponseCode, HeaderInfo.m_ContentLength);

	// fetch content
	if(HeaderInfo.m_ResponseCode == RESPONSECODE_OK && HeaderInfo.m_ContentLength >= 0)
	{
		int DataSize = HeaderInfo.m_ContentLength;
		char *pData = (char *)mem_alloc(DataSize, sizeof(void*));

		// copy a portion of the data we already got
		int HeaderPart = HeaderDataSize - HeaderEnd;
		mem_copy(pData, &aHeader[HeaderEnd], HeaderPart);
		
		int DataOffset = HeaderPart;
		while(DataOffset < DataSize)
		{
			int NumBytes = net_tcp_recv(Socket, pData + DataOffset, DataSize - DataOffset);
			if(NumBytes <= 0)
				break;
			DataOffset += NumBytes;
		}

		if(DataOffset == DataSize)
		{
			pOrder->m_pData = pData;
			pOrder->m_DataSize = DataSize;
		}
		else
		{
			mem_free(pData);
		}

	}

	// clean everything up
	net_tcp_close(Socket);

	return pOrder->m_pData != 0x0;
}

