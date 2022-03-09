// ChatDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "HoldingHands.h"
#include "ChatDlg.h"
#include "afxdialogex.h"
#include "ChatSrv.h"

// CChatDlg 对话框

IMPLEMENT_DYNAMIC(CChatDlg, CDialogEx)

CChatDlg::CChatDlg(CChatSrv*pHandler, CWnd* pParent /*=NULL*/)
	: CDialogEx(CChatDlg::IDD, pParent)
	, m_Msg(_T(""))
{
	m_pHandler = pHandler;
}

CChatDlg::~CChatDlg()
{
}

void CChatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT2, m_Msg);
	DDX_Control(pDX, IDC_EDIT1, m_MsgList);
}


BEGIN_MESSAGE_MAP(CChatDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDOK, &CChatDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CChatDlg 消息处理程序


void CChatDlg::OnClose()
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	if (m_pHandler)
	{
		m_pHandler->Disconnect();
	}
}


void CChatDlg::OnBnClickedOk()
{
	UpdateData();
	if (m_Msg.GetLength() == 0)
		return;
	if (m_pHandler)
	{
		m_pHandler->SendMsg(m_Msg.GetBuffer());
		//把自己的消息显示到屏幕上.
		CString MyMsg;
		MyMsg.Format(L"[me]:%s\r\n", m_Msg.GetBuffer());

		m_MsgList.SetSel(-1);
		m_MsgList.ReplaceSel(MyMsg);
	}
	m_Msg = L"";
	UpdateData(FALSE);
}


BOOL CChatDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	char szIP[128] = { 0 };
	USHORT uPort = 0;
	ASSERT(m_pHandler != NULL);
	m_pHandler->GetPeerName(szIP, uPort);

	CString Title;
	Title.Format(L"[%s] Chat ",CA2W(szIP).m_szBuffer);
	SetWindowText(Title);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}
