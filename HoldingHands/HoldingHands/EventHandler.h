#pragma once
#include <afx.h>
#include <WinSock2.h>
#include <MSWSock.h>

class CClientContext;
class CEventHandler
{
private:
	friend class CManager;
	friend class CClientContext;
	friend class CIOCPServer;
	//Handler不应清理这两个.应该由它自己清理自己
	CClientContext*	m_pClient;				//Client
	DWORD			m_Identity;				//用来标识不同的功能
public:
	/*
	1.Event 事件
	2.总长度
	3.已读取长度
	4.缓冲区
	*/
	//所有功能模块调用下面这几个函数就可以了。不要管CIOCPClient
	BOOL	Send(WORD Event, char*data, int len);
	
	//所有模块应该在该函数里面就把相关资源清理调.析构函数可能不会被调用

	virtual void OnClose();					//当socket断开的时候调用这个函数
	virtual void OnConnect();				//当socket连接的时候调用这个函数
	//有数据到达的时候调用这两个函数.
	virtual void OnReadPartial(WORD Event, DWORD Total, DWORD nRead, char*Buffer);
	virtual void OnReadComplete(WORD Event, DWORD Total, DWORD nRead, char*Buffer);
	//有数据发送完毕后调用这两个函数
	virtual void OnWritePartial(WORD Event, DWORD Total, DWORD nWrite, char*Buffer);
	virtual void OnWriteComplete(WORD Event, DWORD Total, DWORD nWrite, char*Buffer);

	//断开连接
public:
	void Disconnect();
	//
	SOCKET GetSocket();
	void GetPeerName(char* Addr, USHORT&Port);
	void GetSockName(char* Addr, USHORT&Port);

	CEventHandler(DWORD Identity);
	~CEventHandler();
};