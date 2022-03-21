#include "Packet.h"

CPacket::CPacket()
{
	//初始长度应该刚好可以存下一个Header
	m_pBuffer = (char*)_aligned_malloc(PACKET_HEADER_LEN,alignment);
	m_dwBufferLen = PACKET_HEADER_LEN;
	memset(m_pBuffer, 0, sizeof(m_pBuffer));
}

CPacket::~CPacket()
{
	if (m_pBuffer)
	{
		_aligned_free(m_pBuffer);
		m_pBuffer = NULL;
	}
	m_dwBufferLen = 0;
}


//分配Buffer
BOOL CPacket::AllocateMem(DWORD BodyLen)
{
	//缓冲区不够了,就重新分配
	DWORD NewSize = BodyLen + PACKET_HEADER_LEN;
	if (NewSize > MAX_BUFFER_SIZE)
		return FALSE;//太大了,不管了.

	if (NewSize > m_dwBufferLen)
	{
		//realloc memory
		char*pNewBuffer;
		pNewBuffer = (char*)_aligned_realloc(m_pBuffer, NewSize,alignment);
		if (pNewBuffer == NULL)
		{
			return FALSE;
		}
		m_pBuffer = pNewBuffer;
		m_dwBufferLen = NewSize;
	}
	return m_pBuffer != NULL;
}

