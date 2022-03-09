#pragma once
#include "EventHandler.h"



#define CHAT		('C'|('H'<<8)|('A'<<16)|('T'<<24))


#define CHAT_INIT		(0xaa00)			//client---svr
#define CHAT_BEGIN		(0xaa01)			//svr---client
#define CHAT_MSG		(0xaa02)			//p2p



class CChatDlg;

class CChatSrv :
	public CEventHandler
{
public:
	CChatDlg*	m_pDlg;
	WCHAR		m_szNickName[128];



	void OnClose();					//当socket断开的时候调用这个函数
	void OnConnect();				//当socket连接的时候调用这个函数
	//有数据到达的时候调用这两个函数.
	void OnReadPartial(WORD Event, DWORD Total, DWORD nRead, char*Buffer);
	void OnReadComplete(WORD Event, DWORD Total, DWORD nRead, char*Buffer);
	//有数据发送完毕后调用这两个函数
	void OnWritePartial(WORD Event, DWORD Total, DWORD nWrite, char*Buffer);
	void OnWriteComplete(WORD Event, DWORD Total, DWORD nWrite, char*Buffer);



	void OnChatInit(DWORD dwRead, char*szBuffer);
	void OnChatMsg(DWORD dwRead, char*szbuffer);


	void SendMsg(WCHAR*szMsg);
	CChatSrv(DWORD dwIdentity);
	~CChatSrv();
};

