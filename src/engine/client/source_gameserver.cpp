
#include <engine/message.h>
#include <engine/shared/protocol.h>

#include "source_gameserver.h"

CSource_GameServer::CChunk *CSource_GameServer::CChunk::Create(const void *pData, unsigned DataSize)
{
	CChunk *pChunk = (CChunk *)(new char[sizeof(CChunk)+ DataSize]);
	if(pData)
		mem_copy(pChunk->m_aData, pData, DataSize);
	pChunk->m_DataSize = DataSize;
	return pChunk;
}


void CSource_GameServer::SendMsg(CMsgPacker *pMsg)
{
	m_lpOutputChunks.push(CChunk::Create(pMsg->Data(), pMsg->Size()));
}

void CSource_GameServer::SendNextFetch()
{
	CMsgPacker Msg(NETMSG_REQUEST_RES_DATA);
	Msg.AddInt(m_pOrder->m_pResource->NameHash());
	Msg.AddInt(m_pOrder->m_pResource->ContentHash());
	Msg.AddInt(m_DataOffset);
	SendMsg(&Msg);
}

bool CSource_GameServer::ProcessChunk(CChunk *pChunk)
{
	CUnpacker Unpacker;
	Unpacker.Reset(pChunk->m_aData, pChunk->m_DataSize);

	// unpack msgid and system flag
	// TODO: hate that this has to be done here as well
	int Msg = Unpacker.GetInt();
	int Sys = Msg&1;
	Msg >>= 1;

	if(!Sys) // should never happen
		return false;

	switch(Msg)
	{
	case NETMSG_RES_DATA:
		{
			int ContentHash = Unpacker.GetInt();
			int ContentSize = Unpacker.GetInt();
			unsigned BlockOffset = (unsigned)Unpacker.GetInt();
			int BlockSize = Unpacker.GetInt();
			const unsigned char *pBlockData = Unpacker.GetRaw(BlockSize);

			// TODO: loads of sane checks here! LOADS OF EM!
			(void)ContentHash;

			// check for errors
			if(Unpacker.Error() || BlockSize <= 0/* || MapCRC != m_MapdownloadCrc || Chunk != m_MapdownloadChunk || !m_MapdownloadFile*/)
				return false; // TODO: must bail nicly, this isn't

			if(ContentSize > 16*1024*1024 || ContentSize <= 0)
				return false; // TODO: must bail nicly, this isn't

			if(BlockOffset != m_DataOffset)
				return false; // TODO: must bail nicly, this isn't

			// allocate the data if needed
			if(m_pOrder->m_pData == 0x0)
			{
				m_pOrder->m_pData = mem_alloc(ContentSize, sizeof(void*));
				m_pOrder->m_DataSize = ContentSize;
			}

			if(m_DataOffset + BlockSize > m_pOrder->m_DataSize)
				return false;

			mem_copy(((char *)m_pOrder->m_pData) + m_DataOffset, pBlockData, BlockSize);

			// advance offset
			m_DataOffset += BlockSize;

			if(m_DataOffset == m_pOrder->m_DataSize)
			{
				// TODO: crc check data
				unsigned HashValue = hash_crc32(0, m_pOrder->m_pData, m_pOrder->m_DataSize);
				if(HashValue != (unsigned)ContentHash)
					return false;
				m_Done = true;
			}
			else
				SendNextFetch();
		} break;
	}

	return true;
}

bool CSource_GameServer::Load(CLoadOrder *pOrder)
{
	if(!m_Active)
		return false;

		// setup and send initial fetch
	m_pOrder = pOrder;
	m_DataOffset = 0;
	m_Done = false;
	SendNextFetch();
	//dbg_msg("resources", "[%s] starting transfer of '%s'", Name(), m_pOrder->m_pResource->Name());

	// send chunks
	while(true)
	{
		m_Activity.wait();

		if(!m_Active)
			return false;

		bool Error = false;

		// pump input network messages
		while(m_lpInputChunks.size())
		{
			CChunk *pChunk = m_lpInputChunks.pop();
			if(!ProcessChunk(pChunk))
				Error = true;
			delete pChunk;
		}

		// check for errors
		if(Error)
			return false;

		// check if we are done
		if(m_Done)
			return true;
	}

	return false;
}


CSource_GameServer::CSource_GameServer()
: CSource("gameserver")
{
	m_Active = 0;
}

void CSource_GameServer::SetActive(bool Active)
{
	m_Active = Active;
	m_Activity.signal();
}

void CSource_GameServer::QueueChunk(const void *pData, unsigned DataSize)
{
	CChunk *pChunk = CChunk::Create(pData, DataSize);
	m_lpInputChunks.push(pChunk);
	m_Activity.signal();
}

CSource_GameServer::CChunk *CSource_GameServer::PopOutputChunk()
{
	if(m_lpOutputChunks.size() == 0)
		return 0;
	return m_lpOutputChunks.pop();
}

