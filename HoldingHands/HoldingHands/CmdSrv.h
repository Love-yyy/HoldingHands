#pragma once
#include "EventHandler.h"
#define CMD				('C'|('M')<<8|('D')<<16)

//0成功,-1失败
#define CMD_BEGIN		(0xcad0)

#define CMD_COMMAND		(0xcad1)
#define CMD_RESULT		(0xcad2)

class CCmdWnd;
class CCmdSrv :
	public CEventHandler
{
private:
	CCmdWnd *	m_pWnd;

public:
	void OnClose();					//当socket断开的时候调用这个函数
	void OnConnect();				//当socket连接的时候调用这个函数
	//有数据到达的时候调用这两个函数.
	void OnReadPartial(WORD Event, DWORD Total, DWORD nRead, char*Buffer);
	void OnReadComplete(WORD Event, DWORD Total, DWORD nRead, char*Buffer);
	//有数据发送完毕后调用这两个函数
	void OnWritePartial(WORD Event, DWORD Total, DWORD nWrite, char*Buffer);
	void OnWriteComplete(WORD Event, DWORD Total, DWORD nWrite, char*Buffer);
	//

	void OnCmdBegin(DWORD dwStatu);
	void OnCmdResult(char*szBuffer);
	CCmdSrv(DWORD dwIdentity);
	~CCmdSrv();
};

