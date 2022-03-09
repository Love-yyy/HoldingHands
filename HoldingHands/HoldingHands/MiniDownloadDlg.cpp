// MiniDownloadDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "HoldingHands.h"
#include "MiniDownloadDlg.h"
#include "afxdialogex.h"
#include "MiniDownloadSrv.h"

// CMiniDownloadDlg 对话框

IMPLEMENT_DYNAMIC(CMiniDownloadDlg, CDialogEx)

CMiniDownloadDlg::CMiniDownloadDlg(CMiniDownloadSrv* pHandler, CWnd* pParent /*=NULL*/)
	: CDialogEx(CMiniDownloadDlg::IDD, pParent)
{
	m_pHandler = pHandler;
	m_ullTotalSize = 0;
	m_ullFinishedSize = 0;

	char szIP[128];
	USHORT uPort;
	m_pHandler->GetPeerName(szIP, uPort);
	m_IP = CA2W(szIP);
}

CMiniDownloadDlg::~CMiniDownloadDlg()
{
}

void CMiniDownloadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DOWNLOAD_PROGRESS, m_Progress);
}


BEGIN_MESSAGE_MAP(CMiniDownloadDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CMiniDownloadDlg::OnBnClickedOk)
	ON_MESSAGE(WM_MNDD_FILEINFO, OnGetFileInfoReply)
	ON_MESSAGE(WM_MNDD_DOWNLOAD_RESULT, OnDownloadReply)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CMiniDownloadDlg 消息处理程序


void CMiniDownloadDlg::OnBnClickedOk()
{
}

LRESULT CMiniDownloadDlg::OnGetFileInfoReply(WPARAM wParam, LPARAM lParam)
{
	MnddFileInfo*pFileInfo = (MnddFileInfo*)wParam;
	CString Text;

	switch (pFileInfo->dwStatu)
	{
	case MNDD_STATU_OK:
		m_ullTotalSize = pFileInfo->dwFileSizeHi;
		m_ullTotalSize <<= 32;
		m_ullTotalSize |= pFileInfo->dwFileSizeLo;
		break;
	case MNDD_STATU_UNKNOWN_FILE_SIZE:
		m_ullTotalSize = -1;//假设非常大.
		m_Progress.SetMarquee(TRUE, 30);//进度条设置为未定义.
		break;
	case MNDD_STATU_ANALYSE_URL_FAILED:
		MessageBox(L"Analyse url failed!", L"Error");
		break;
	case MNDD_STATU_INTERNETOPEN_FAILED:
		MessageBox(L"Internet Open Failed!", L"Error");
		break;
	case MNDD_STATU_INTERNETCONNECT_FAILED:
		MessageBox(L"Internet Connect Failed!", L"Error");
		break;
	case MNDD_STATU_OPENREMOTEFILE_FILED:
		MessageBox(L"Open Remote File Failed!", L"Error");
		break;
	case MNDD_STATU_UNSUPPORTED_PROTOCOL:
		MessageBox(L"Unsupported Protocol!", L"Error");
		break;
	case MNDD_STATU_HTTPSENDREQ_FAILED:
		MessageBox(L"Http Send Request Failed!", L"Error");
		break;
	case MNDD_STATU_OPENLOCALFILE_FAILED:
		MessageBox(L"Open Local File Failed!", L"Error");
		break;
	}
	return 0;
}
LRESULT CMiniDownloadDlg::OnDownloadReply(WPARAM wParam, LPARAM lParam)
{
	DownloadResult*pResult = (DownloadResult*)wParam;
	CString Title;
	//更新下载进度.
	m_ullFinishedSize += pResult->dwWriteSize;
	WCHAR* MemUnits[] =
	{
		L"Byte",
		L"KB",
		L"MB",
		L"GB",
		L"TB"
	};
	//
	ULONGLONG ullFinished = m_ullFinishedSize, ullTotal = m_ullTotalSize;
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
	//
	switch (pResult->dwStatu)
	{
	case 0://OK
	case 3:
		if (m_ullTotalSize == -1)
		{
			Title.Format(L"[%s] Totalo:Unknown Download:%d %s", m_IP.GetBuffer(), dwFinished, MemUnits[MemUnitIdx1]);
		}
		else
		{
			Title.Format(L"[%s] FileSize:%d %s  Download:%d %s", m_IP.GetBuffer(), dwTotal, MemUnits[MemUnitIdx2], dwFinished, MemUnits[MemUnitIdx1]);
			m_Progress.SetPos(m_ullFinishedSize * 100 / m_ullTotalSize);
		}
		SetWindowText(Title);
	
		if (pResult->dwStatu == 3)
		{
			MessageBox(L"Download Finished!", m_IP.GetBuffer());
		}
		break;
	case 1:
		MessageBox(L"Internet Read File Failed!", L"Error");
		break;
	case 2:
		MessageBox(L"Write File Failed!", L"Error");
		break;
	}
	return 0;
}

void CMiniDownloadDlg::OnClose()
{
	if (m_pHandler)
	{
		m_pHandler->Disconnect();
	}
}


BOOL CMiniDownloadDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	CString Title;
	Title.Format(L"[%s] Getting File Size...", m_IP.GetBuffer());
	SetWindowText(Title);

	m_Progress.SetRange(0, 100);
	m_Progress.SetPos(0);
	return TRUE;  // return TRUE unless you set the focus to a control
}
