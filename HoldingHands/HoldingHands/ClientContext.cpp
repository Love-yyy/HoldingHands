#include "stdafx.h"
#include "ClientContext.h"
#include "Packet.h"
#include "EventHandler.h"
#include "IOCPServer.h"
BOOL CClientContext::BindHandler(CEventHandler*pHandler)
{
	//已经绑定了Handler;
	if (m_pHandler != NULL)
		return FALSE;
	m_pHandler = pHandler;
	pHandler->m_pClient = this;
	return TRUE;
}

BOOL CClientContext::UnbindHandler()
{
	if (m_pHandler == NULL)
		return FALSE;
	m_pHandler->m_pClient = NULL;
	m_pHandler = NULL;
	return TRUE;
}

CClientContext::CClientContext(SOCKET ClientSocket, CIOCPServer *pServer)
{
	//
	m_pServer = pServer;
	//Event Handler
	m_Identity = 0;
	m_pHandler = NULL;
	//SOCKET
	m_ClientSocket = ClientSocket;
	//-------------------------------------------------------------------------------------------
	m_wsaReadBuf.len = 0;
	m_wsaReadBuf.buf = 0;
	m_dwRead = 0;
	//
	m_wsaWriteBuf.buf = 0;
	m_wsaWriteBuf.len = 0;
	m_dwWrite = 0;
	//-------------------------------------------------------------------------------------------
	m_PosInList = 0;
	InitializeCriticalSectionAndSpinCount(&m_csCheck,3000);
	m_SendPacketOver = CreateEvent(0, FALSE, TRUE, NULL);
}


CClientContext::~CClientContext()
{
	if (m_ClientSocket != INVALID_SOCKET)
	{
		closesocket(m_ClientSocket);
		m_ClientSocket = INVALID_SOCKET;
	}
	if (m_pHandler)
	{
		//一般不会执行这里.(在释放CClientContext之前已经解绑)
		delete m_pHandler;
		m_pHandler = NULL;
	}
	if (m_SendPacketOver)
	{
		CloseHandle(m_SendPacketOver);
		m_SendPacketOver = NULL;
	}
	DeleteCriticalSection(&m_csCheck);
}



int CClientContext::SendPacket(DWORD command, char*data, int len)
{
	for (;;)
	{
		//OnClose之后，会触发事件，这里就进不去了。
		DWORD dwResult = WaitForSingleObject(m_SendPacketOver, 500);
		if (dwResult == WAIT_OBJECT_0)
		{
			if (m_ClientSocket == INVALID_SOCKET)
			{
				SetEvent(m_SendPacketOver);
				return -1;
			}
			break;
		}
		if (m_ClientSocket == INVALID_SOCKET)
			return -1;
	}

	if (FALSE == m_WritePacket.AllocateMem(len))
	{
		SetEvent(m_SendPacketOver);//发送失败
		return -1;
	}
	//Set Header
	m_WritePacket.SetHeader(len, command);
	//Copy Data.
	memcpy(m_WritePacket.GetBody(), data, len);
	m_dwWrite = 0;
	m_wsaWriteBuf.buf = m_WritePacket.GetBuffer() + m_dwWrite;
	m_wsaWriteBuf.len = m_WritePacket.GetBodyLen() + PACKET_HEADER_LEN - m_dwWrite;
	//服务器投递发送请求.
	m_pServer->PostWrite(this);	
	return 0;
}

//断开一个连接
void CClientContext::Disconnect()
{
	EnterCriticalSection(&m_csCheck);
	if (m_ClientSocket != INVALID_SOCKET)
	{
		closesocket(m_ClientSocket);
		m_ClientSocket = INVALID_SOCKET;
	}
	LeaveCriticalSection(&m_csCheck);
}

void CClientContext::GetPeerName(char* Addr, USHORT&Port)
{
	Port = m_PeerPort;
	strcpy(Addr, m_szPeerAddr);
	
}
void CClientContext::GetSockName(char* Addr, USHORT&Port)
{
	Port = m_SockPort;
	strcpy(Addr, m_szSockAddr);
}