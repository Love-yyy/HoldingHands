#pragma once
#include <WinSock2.h>
#include <MSWSock.h>
/*
	重写下面三个函数;
	virtual void OnClose() = 0;
	virtual void OnReadPartial(CPacket*pPacket) = 0;
	virtual void OnReadComplete(CPacket*pPacket) = 0;
*/

class CIOCPClient;
class CEventHandler
{
private:
	friend class CIOCPClient;
	friend class CManager;
	//Handler不应清理这两个.应该由它自己清理自己;
	CIOCPClient*m_pClient;				//Client;
	DWORD		m_Identity;				//用来标识不同的功能;
public:
	/*
	1.Event 事件;
	2.总长度;
	3.已读取长度;
	4.缓冲区;
	*/
	//所有功能模块调用下面这几个函数就可以了。不要管CIOCPClient;
	
	virtual void OnClose();						//所有模块应该在该函数里面就把相关资源清理调.因为相关的析构函数不会被调用;
	virtual void OnConnect();
	
	virtual void OnReadPartial(WORD Event, DWORD Total, DWORD dwRead, char*Buffer);
	virtual void OnReadComplete(WORD Event, DWORD Total, DWORD dwRead, char*Buffer);
	virtual void OnWritePartial(WORD Event, DWORD dwTotal, DWORD dwWrite, char*Buffer);
	virtual void OnWriteComplete(WORD Event, DWORD dwTotal, DWORD dwWrite, char*Buffer);

	void		Send(WORD Event, char*data, int len);
	void		Disconnect();

	void GetPeerName(char* Addr, USHORT&Port);
	void GetSockName(char* Addr, USHORT&Port);
	void GetSrvName(char*Addr, USHORT&Port);

public:
	
	CEventHandler(DWORD Identity);
	~CEventHandler();
};

