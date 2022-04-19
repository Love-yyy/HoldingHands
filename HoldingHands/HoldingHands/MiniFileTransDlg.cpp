// MiniFileTransDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "HoldingHands.h"
#include "MiniFileTransDlg.h"
#include "afxdialogex.h"
#include "resource.h"
#include "MiniFileTransSrv.h"
// CMiniFileTransDlg 对话框

IMPLEMENT_DYNAMIC(CMiniFileTransDlg, CDialogEx)

CMiniFileTransDlg::CMiniFileTransDlg(CMiniFileTransSrv*pHandler, CWnd* pParent /*=NULL*/)
	: CDialogEx(CMiniFileTransDlg::IDD, pParent)
{
	m_pHandler = pHandler;

	m_ullTotalSize = 0;
	m_ullFinishedSize = 0;

	m_dwTotalCount = 0;
	m_dwFinishedCount = 0;
	m_dwFailedCount = 0;

	char szIP[128] = { 0 };
	USHORT uPort;
	m_pHandler->GetPeerName(szIP, uPort);
	m_IP = CA2W(szIP);
}

CMiniFileTransDlg::~CMiniFileTransDlg()
{
}

void CMiniFileTransDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TRANS_PROGRESS, m_Progress);
	DDX_Control(pDX, IDC_EDIT1, m_TransLog);
}


BEGIN_MESSAGE_MAP(CMiniFileTransDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CMiniFileTransDlg::OnBnClickedOk)
	ON_WM_CLOSE()
	ON_MESSAGE(WM_MNFT_TRANS_INFO, OnTransInfo)
	ON_MESSAGE(WM_MNFT_FILE_TRANS_BEGIN, OnTransFileBegin)
	ON_MESSAGE(WM_MNFT_FILE_TRANS_FINISHED, OnTransFileFinished)
	ON_MESSAGE(WM_MNFT_FILE_DC_TRANSFERRED, OnTransFileDC)
	ON_MESSAGE(WM_MNFT_TRANS_FINISHED, OnTransFinished)
END_MESSAGE_MAP()


// CMiniFileTransDlg 消息处理程序


void CMiniFileTransDlg::OnBnClickedOk()
{
	//防止退出.
}


void CMiniFileTransDlg::OnClose()
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	if (m_pHandler != NULL)
	{
		//断开连接
		m_pHandler->Disconnect();
	}
	//不关闭.断开后delete 会自动关闭.
}


BOOL CMiniFileTransDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// TODO:  在此添加额外的初始化
	//
	m_Progress.SetRange(0, 100);
	m_Progress.SetPos(0);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}

LRESULT CMiniFileTransDlg::OnTransInfo(WPARAM wParam, LPARAM lParam)
{
	CMiniFileTransSrv::MNFT_Trans_Info*pTransInfo = (CMiniFileTransSrv::MNFT_Trans_Info*)wParam;
	m_ullTotalSize = pTransInfo->TotalSizeHi;
	m_ullTotalSize <<= 32;
	m_ullTotalSize |= pTransInfo->TotalSizeLo;

	m_dwTotalCount = pTransInfo->dwFileCount;
	//
	CString Text;
	if (lParam == MNFT_DUTY_RECEIVER)
		Text.Format(L"[%s] Downloading...", m_IP.GetBuffer());
	else
		Text.Format(L"[%s] Uploading...", m_IP.GetBuffer());
	SetWindowText(Text);

	Text.Format(L"%d / %d", m_dwFinishedCount, m_dwTotalCount);
	GetDlgItem(IDC_COUNT_PROGRESS)->SetWindowTextW(Text);
	return 0;
}
LRESULT CMiniFileTransDlg::OnTransFileBegin(WPARAM wParam, LPARAM lParam)
{
	CMiniFileTransSrv::FileInfo*pFile = (CMiniFileTransSrv::FileInfo*)wParam;
	CString Text;
	Text.Format(L"Processing:%s", pFile->RelativeFilePath);
	m_TransLog.SetSel(-1);
	m_TransLog.ReplaceSel(Text);

	GetDlgItem(IDC_CURRENT_FILE)->SetWindowTextW(pFile->RelativeFilePath);
	return 0;
}

LRESULT CMiniFileTransDlg::OnTransFileFinished(WPARAM wParam, LPARAM lParam)
{
	m_TransLog.SetSel(-1);
	CString Text = L"   -OK\r\n";

	if (wParam != MNFT_STATU_SUCCESS)
	{
		m_dwFailedCount++;
		Text = L"   -Failed\r\n";
	}
	//记录结果.
	m_TransLog.ReplaceSel(Text);
	m_dwFinishedCount++;
	Text.Format(L"%d / %d", m_dwFinishedCount, m_dwTotalCount);
	GetDlgItem(IDC_COUNT_PROGRESS)->SetWindowTextW(Text);
	return 0;
}

LRESULT CMiniFileTransDlg::OnTransFileDC(WPARAM wParam, LPARAM lParam)
{
	WCHAR* MemUnits[] =
	{
		L"Byte",
		L"KB",
		L"MB",
		L"GB",
		L"TB"
	};

	m_ullFinishedSize += wParam;
	ULONGLONG ullFinished = m_ullFinishedSize,ullTotal = m_ullTotalSize;
	int MemUnitIdx1 = 0, MemUnitIdx2 = 0;
	int MaxIdx = (sizeof(MemUnits) / sizeof(MemUnits[0])) - 1;
	//Get FinishedSize Unit.
	while (ullFinished > 1024 && MemUnitIdx1 < MaxIdx)
	{
		MemUnitIdx1++;
		ullFinished >>= 10;
	}
	DWORD dwFinished = (ullFinished & 0xffffffff);
	++dwFinished;
	//Get TotalSize Unit
	while (ullTotal > 1024 && MemUnitIdx2 < MaxIdx)
	{
		MemUnitIdx2++;
		ullTotal >>= 10;
	}
	DWORD dwTotal = (ullTotal & 0xffffffff);
	++dwTotal;
	//SetText
	CString Text;
	Text.Format(L"%d %s/%d %s", dwFinished, MemUnits[MemUnitIdx1], dwTotal, MemUnits[MemUnitIdx2]);
	GetDlgItem(IDC_SZIE_PROGRESS)->SetWindowTextW(Text);
	//SetProgressorPos;
	m_Progress.SetPos(m_ullFinishedSize * 100 / m_ullTotalSize);
	return 0;
}

LRESULT CMiniFileTransDlg::OnTransFinished(WPARAM wParam, LPARAM lParam)
{
	CString Text;
	Text.Format(L"[%s] Transfer Complete!", m_IP.GetBuffer());
	SetWindowText(Text);
	Text.Format(L"Totoal File Count:%d \r\nSuccess:%d \r\nFailed:%d \r\n", m_dwTotalCount, m_dwFinishedCount - m_dwFailedCount, m_dwFailedCount);
	MessageBox(Text,L"Transfer Complete!");
	return 0;
}
