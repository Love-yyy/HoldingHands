#pragma once

class CPacket;
class CIOCPServer;
class CClientContext;

enum COMPLETED_PACKET_TYPE
{
	PACKET_READ_COMPLETED = 0x01,
	PACKET_READ_PARTIAL,
	PACKET_WRITE_COMPLETED,
	PACKET_WRITE_PARTIAL,
	PACKET_CLIENT_CONNECT,
	PACKET_CLIENT_DISCONNECT,
	PACKET_ACCEPT_ABORT
};

#define WM_SOCKET_CONNECT (WM_USER+201)
#define WM_SOCKET_CLOSE (WM_USER+202)

class CManager
{
private:
	CIOCPServer* m_pServer;
public:
	static BOOL HandlerInit(CClientContext*pClientContext, DWORD Identity);
	static BOOL HandlerTerm(CClientContext*pClientContext, DWORD Identity);

	void ProcessCompletedPacket(int type,CClientContext*pContext,CPacket*pPacket);
	CManager(CIOCPServer*pServer);
	~CManager();
};

