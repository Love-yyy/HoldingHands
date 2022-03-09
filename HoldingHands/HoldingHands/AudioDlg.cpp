// AudioDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "HoldingHands.h"
#include "AudioDlg.h"
#include "afxdialogex.h"
#include "AudioSrv.h"

// CAudioDlg 对话框

IMPLEMENT_DYNAMIC(CAudioDlg, CDialogEx)

CAudioDlg::CAudioDlg(CAudioSrv*pHandler, CWnd* pParent /*=NULL*/)
	: CDialogEx(CAudioDlg::IDD, pParent)
{
	m_pHandler = pHandler;
}

CAudioDlg::~CAudioDlg()
{
}

void CAudioDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAudioDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_MESSAGE(WM_AUDIO_ERROR,OnError)
END_MESSAGE_MAP()


// CAudioDlg 消息处理程序


void CAudioDlg::OnClose()
{
	if (m_pHandler)
	{
		m_pHandler->Disconnect();
	}
}

void CAudioDlg::OnOK()
{
}


void CAudioDlg::OnCancel()
{
}


BOOL CAudioDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	char IP[128] = { 0 };
	USHORT uPort;
	m_pHandler->GetPeerName(IP, uPort);
	//
	CString Title;
	Title.Format(L"[%s] Microphone", CA2W(IP).m_szBuffer);
	SetWindowText(Title);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}

LRESULT CAudioDlg::OnError(WPARAM wParam, LPARAM lParam)
{
	WCHAR*szError = (WCHAR*)wParam;
	MessageBox(szError, L"Error");
	return 0;
}