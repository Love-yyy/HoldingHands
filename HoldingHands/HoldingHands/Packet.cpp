#include "stdafx.h"
#include "Packet.h"

CPacket::CPacket()
{
	//��ʼ����Ӧ�øպÿ��Դ���һ��Header
	m_pBuffer = (char*)_aligned_malloc(PACKET_HEADER_LEN, alignment);
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


//����Buffer
BOOL CPacket::AllocateMem(DWORD BodyLen)
{
	//������������,�����·���
	DWORD NewSize = BodyLen + PACKET_HEADER_LEN;
	if (NewSize > MAX_BUFFER_SIZE)
		return FALSE;//̫����,������.

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
	return m_pBuffer!=NULL;
}
