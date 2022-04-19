#include "EventHandler.h"
#include "IOCPClient.h"
#include "Packet.h"

CEventHandler::CEventHandler(DWORD Identity)
{
	m_Identity = Identity;
}

CEventHandler::~CEventHandler()
{
}

void CEventHandler::Send(WORD Event, char*data, int len)
{
	assert(m_pClient != NULL);
	//·¢ËÍ
	m_pClient->SendPacket(Event, data, len);
}
void CEventHandler::Disconnect()
{
	assert(m_pClient != NULL);
	m_pClient->Disconnect();
}
void CEventHandler::GetPeerName(char* Addr, USHORT&Port)
{
	Addr[0] = 0;
	Port = 0;
	assert(m_pClient != NULL);
	m_pClient->GetPeerName(Addr, Port);

}
void CEventHandler::GetSockName(char* Addr, USHORT&Port)
{
	Addr[0] = 0;
	Port = 0;
	assert(m_pClient != NULL);
	m_pClient->GetSockName(Addr, Port);

}
void CEventHandler::GetSrvName(char*Addr, USHORT&Port)
{
	Addr[0] = 0;
	Port = 0;
	assert(m_pClient != NULL);
	m_pClient->GetSrvName(Addr, Port);
}

void CEventHandler::OnConnect(){

}
void CEventHandler::OnClose(){

}
void CEventHandler::OnWritePartial(WORD Event, DWORD dwTotal, DWORD dwWrite, char*Buffer){

}
void CEventHandler::OnWriteComplete(WORD Event, DWORD dwTotal, DWORD dwWrite, char*Buffer){

}
void CEventHandler::OnReadPartial(WORD Event, DWORD Total, DWORD dwRead, char*Buffer){

}
void CEventHandler::OnReadComplete(WORD Event, DWORD Total, DWORD dwRead, char*Buffer){

}