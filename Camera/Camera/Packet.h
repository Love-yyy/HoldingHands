#pragma once
#include <winsock2.h>
#include <MSWSock.h>

#define MAX_BUFFER_SIZE (32*1024*1024)
#define alignment 4
#define PACKET_CHECK					0x534E4942

//高16位记录功能ID，低16位记录每一个功能里面的通知
/*
		修改:
			只需要Event就可以了。
*/

/*
struct CPacketHeader
{
DWORD			m_dwCheck;						//数据校验
DWORD			m_dwBufferLen;					//Buffer长度
DWORD			m_dwCommand;					//远程命令
DWORD			m_reserved;						//保留
};
*/
#define PACKET_HEADER_LEN				16

class CPacket
{
private:
	char* m_pBuffer;					//用来储存Header + Body
	DWORD m_dwBufferLen;				//缓冲区长度
public:

	BOOL			AllocateMem(DWORD BodyLen);

	//校验数据
	inline BOOL		Verify()
	{
		return ((DWORD*)m_pBuffer)[0] == PACKET_CHECK;
	}
	//
	inline char*	GetBody()
	{
		return m_pBuffer + PACKET_HEADER_LEN;
	}

	inline char*	GetBuffer()
	{
		return m_pBuffer;
	}
	inline DWORD	GetBodyLen()
	{
		return ((DWORD*)m_pBuffer)[1];
	}

	inline DWORD	GetCommand()
	{
		return ((DWORD*)m_pBuffer)[2];
	}
	inline BOOL		SetHeader(DWORD BodyLen, DWORD Command)
	{
		if (m_pBuffer == NULL || m_dwBufferLen<PACKET_HEADER_LEN)
		{
			return FALSE;
		}
		((DWORD*)m_pBuffer)[0] = PACKET_CHECK;
		((DWORD*)m_pBuffer)[1] = BodyLen;
		((DWORD*)m_pBuffer)[2] = Command;
		return TRUE;
	}
	CPacket();
	~CPacket();
};

