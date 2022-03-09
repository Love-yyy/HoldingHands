#include "stdafx.h"
#include "FileMgrSearchSrv.h"
#include "FileMgrSearchDlg.h"

CFileMgrSearchSrv::CFileMgrSearchSrv(DWORD dwIdentity) :
CEventHandler(dwIdentity)
{
	m_pDlg = NULL;
}


CFileMgrSearchSrv::~CFileMgrSearchSrv()
{
}

void CFileMgrSearchSrv::OnClose()
{
	if (m_pDlg)
	{
		delete m_pDlg;
		m_pDlg = NULL;
	}
}
void CFileMgrSearchSrv::OnConnect()
{
	m_pDlg = new CFileMgrSearchDlg(this);
	if (m_pDlg->Create(IDD_FM_SEARCH_DLG) == FALSE)
	{
		Disconnect();
		return;
	}
	m_pDlg->ShowWindow(SW_SHOW);
}
//有数据到达的时候调用这两个函数.
void CFileMgrSearchSrv::OnReadPartial(WORD Event, DWORD Total, DWORD nRead, char*Buffer)
{

}
void CFileMgrSearchSrv::OnReadComplete(WORD Event, DWORD Total, DWORD nRead, char*Buffer)
{
	switch (Event)
	{
	case FILE_MGR_SEARCH_FOUND:
		OnFound(Buffer);
		break;
	case FILE_MGR_SEARCH_OVER:
		OnOver();
		break;
	default:
		break;
	}
}
//有数据发送完毕后调用这两个函数
void CFileMgrSearchSrv::OnWritePartial(WORD Event, DWORD Total, DWORD nWrite, char*Buffer)
{

}
void CFileMgrSearchSrv::OnWriteComplete(WORD Event, DWORD Total, DWORD nWrite, char*Buffer)
{

}

void CFileMgrSearchSrv::OnFound(char*Buffer)
{	
	m_pDlg->SendMessage(WM_FILE_MGR_SEARCH_FOUND, (WPARAM)Buffer, 0);
}
void CFileMgrSearchSrv::OnOver()
{
	m_pDlg->SendMessage(WM_FILE_MGR_SEARCH_OVER, 0, 0);
}

void CFileMgrSearchSrv::Search(WCHAR*szParams)
{
	Send(FILE_MGR_SEARCH_SEARCH, (char*)szParams, (sizeof(WCHAR) * (wcslen(szParams) + 1)));
}

void CFileMgrSearchSrv::Stop()
{
	Send(FILE_MGR_SEARCH_STOP, 0, 0);
}