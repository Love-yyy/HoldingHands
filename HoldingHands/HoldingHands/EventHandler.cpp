#include "stdafx.h"
#include "EventHandler.h"
#include "IOCPServer.h"
#include "Packet.h"
#include "ClientContext.h"

CEventHandler::CEventHandler(DWORD Identity)
{
	m_Identity = Identity;
}

CEventHandler::~CEventHandler()
{

}

BOOL CEventHandler::Send(WORD Event, char*data, int len)
{
	CIOCPServer *pServer = CIOCPServer::CreateServer(NULL);

	if (pServer == NULL||m_pClient == NULL || m_Identity == NULL)
		return FALSE;
	//发送
	DWORD command = MakeRemoteCommand(m_Identity, Event);
	return (0 == m_pClient->SendPacket(command, data, len));
}

void CEventHandler::Disconnect()
{
	//断开socket 连接
	m_pClient->Disconnect();
}

SOCKET CEventHandler::GetSocket()
{
	SOCKET s = INVALID_SOCKET;
	if (m_pClient)
	{
		s = m_pClient->m_ClientSocket;
	}
	return s;
}
void CEventHandler::GetPeerName(char* Addr, USHORT&Port)
{
	Addr[0] = 0;
	Port = 0;
	if (m_pClient)
	{
		return m_pClient->GetPeerName(Addr, Port);
	}
}
void CEventHandler::GetSockName(char* Addr, USHORT&Port)
{
	Addr[0] = 0;
	Port = 0;
	if (m_pClient)
	{
		return m_pClient->GetSockName(Addr, Port);
	}
}
 void CEventHandler::OnClose()
{

}
 void CEventHandler::OnConnect()
{

}
//有数据到达的时候调用这两个函数.
void CEventHandler::OnReadPartial(WORD Event, DWORD Total, DWORD nRead, char*Buffer)
{

}
 void CEventHandler::OnReadComplete(WORD Event, DWORD Total, DWORD nRead, char*Buffer)
{

}
//有数据发送完毕后调用这两个函数
 void CEventHandler::OnWritePartial(WORD Event, DWORD Total, DWORD nWrite, char*Buffer)
{

}
 void CEventHandler::OnWriteComplete(WORD Event, DWORD Total, DWORD nWrite, char*Buffer)
{

}
