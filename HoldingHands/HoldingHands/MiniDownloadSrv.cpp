#include "stdafx.h"
#include "MiniDownloadSrv.h"
#include "MiniDownloadDlg.h"

CMiniDownloadSrv::CMiniDownloadSrv(DWORD Identity) :
CEventHandler(Identity)
{
	m_pDlg = NULL;
}


CMiniDownloadSrv::~CMiniDownloadSrv()
{

}

void CMiniDownloadSrv::OnClose()
{
	if (m_pDlg)
	{
		delete m_pDlg;
		m_pDlg = NULL;
	}
}
void CMiniDownloadSrv::OnConnect()
{
	//发送请求.获取文件大小.
	Send(MNDD_GET_FILE_INFO, 0, 0);
	//
	m_pDlg = new CMiniDownloadDlg(this);
	if (FALSE == m_pDlg->Create(IDD_MNDD_DLG))
	{
		Disconnect();
		return;
	}
	m_pDlg->ShowWindow(SW_SHOW);
}

void CMiniDownloadSrv::OnReadPartial(WORD Event, DWORD Total, DWORD dwRead, char*Buffer)
{

}
void CMiniDownloadSrv::OnReadComplete(WORD Event, DWORD Total, DWORD dwRead, char*Buffer)
{
	switch (Event)
	{
	case MNDD_FILE_INFO:
		OnGetFileInfoRpl(dwRead, Buffer);
		break;
	case MNDD_DOWNLOAD_RESULT:
		OnDownloadRpl(dwRead, Buffer);
		break;
	default:
		break;
	}
}

void CMiniDownloadSrv::OnWritePartial(WORD Event, DWORD Total, DWORD dwWrite, char*Buffer)
{

}
void CMiniDownloadSrv::OnWriteComplete(WORD Event, DWORD Total, DWORD dwWrite, char*Buffer)
{

}

void CMiniDownloadSrv::OnDownloadRpl(DWORD dwRead, char*Buffer)
{
	DownloadResult* pResult = (DownloadResult*)Buffer;
	switch (pResult->dwStatu)
	{
	case 0:										//下载成功
		Send(MNDD_DOWNLOAD_CONTINUE, 0, 0);
		break;
	default:
		Send(MNDD_DOWNLOAD_END, 0, 0);			//其他情况,包括正常下载完毕和出现错误,直接终止下载.
		break;
	}
	m_pDlg->SendMessage(WM_MNDD_DOWNLOAD_RESULT, (WPARAM)pResult, 0);
}

void CMiniDownloadSrv::OnGetFileInfoRpl(DWORD dwRead, char*Buffer)
{
	MnddFileInfo*pFileInfo = (MnddFileInfo*)Buffer;
	switch (pFileInfo->dwStatu)
	{
	case MNDD_STATU_OK:	//ok
	case MNDD_STATU_UNKNOWN_FILE_SIZE://大小未知.
		Send(MNDD_DOWNLOAD_CONTINUE, 0, 0);//开始下载
		break;
	default:
		Send(MNDD_DOWNLOAD_END, 0, 0);		//终止下载.
		break;
	}
	m_pDlg->SendMessage(WM_MNDD_FILEINFO, (WPARAM)pFileInfo, 0);
}