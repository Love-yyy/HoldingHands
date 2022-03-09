// FileMgrSearchDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "HoldingHands.h"
#include "FileMgrSearchDlg.h"
#include "afxdialogex.h"
#include "FileManagerDlg.h"
#include "FileMgrSearchSrv.h"
// CFileMgrSearchDlg 对话框

IMPLEMENT_DYNAMIC(CFileMgrSearchDlg, CDialogEx)

CFileMgrSearchDlg::CFileMgrSearchDlg(CFileMgrSearchSrv*pHandler,CWnd* pParent /*=NULL*/)
	: CDialogEx(CFileMgrSearchDlg::IDD, pParent)
	, m_TargetName(_T(""))
	, m_StartLocation(_T(""))
{
	m_pHandler = pHandler;
}

CFileMgrSearchDlg::~CFileMgrSearchDlg()
{
}

void CFileMgrSearchDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_TargetName);
	DDX_Text(pDX, IDC_EDIT3, m_StartLocation);
	DDX_Control(pDX, IDC_LIST1, m_ResultList);
}


BEGIN_MESSAGE_MAP(CFileMgrSearchDlg, CDialogEx)
	ON_MESSAGE(WM_FILE_MGR_SEARCH_FOUND,OnFound)
	ON_MESSAGE(WM_FILE_MGR_SEARCH_OVER, OnOver)
	ON_BN_CLICKED(IDOK, &CFileMgrSearchDlg::OnBnClickedOk)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CFileMgrSearchDlg::OnNMDblclkList1)
END_MESSAGE_MAP()


// CFileMgrSearchDlg 消息处理程序


BOOL CFileMgrSearchDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_ResultList.InsertColumn(0, L"Name", LVCFMT_LEFT, 130);
	m_ResultList.InsertColumn(1, L"Location", LVCFMT_LEFT, 370);
	//
	m_ResultList.SetImageList(CFileManagerDlg::pLargeImageList, LVSIL_NORMAL);
	m_ResultList.SetImageList(CFileManagerDlg::pSmallImageList, LVSIL_SMALL);
	//

	m_ResultList.SetExtendedStyle((~ LVS_EX_CHECKBOXES)&(m_ResultList.GetExStyle() | LVS_EX_FULLROWSELECT));
	char szIP[128];
	USHORT uPort;
	CString Title;
	m_pHandler->GetPeerName(szIP, uPort);
	Title.Format(L"[%s] Search", CA2W(szIP).m_szBuffer);
	SetWindowText(Title);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}

struct FindFile
{
	DWORD dwFileAttribute;
	WCHAR szFileName[2];
};

LRESULT CFileMgrSearchDlg::OnFound(WPARAM wParam, LPARAM lParam)
{
	FindFile*pFindFile = (FindFile*)wParam;
	WCHAR*szLocation = pFindFile->szFileName;
	WCHAR*szFileName = wcsstr(pFindFile->szFileName, L"\n");
	if (szFileName == NULL)
		return 0;
	*szFileName++ = NULL;

	SHFILEINFO si = { 0 };
	SHGetFileInfo(szFileName, pFindFile->dwFileAttribute, &si, sizeof(si), SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX);

	int idx = m_ResultList.GetItemCount();
	m_ResultList.InsertItem(idx, szFileName,si.iIcon);
	m_ResultList.SetItemText(idx, 1, szLocation);
	return 0;
}


LRESULT CFileMgrSearchDlg::OnOver(WPARAM wParam, LPARAM lParam)
{
	char szIP[128];
	USHORT uPort;

	CWnd*pButton = GetDlgItem(IDOK);
	pButton->SetWindowTextW(L"Search");
	m_pHandler->GetPeerName(szIP, uPort);
	
	CString Title;
	Title.Format(L"[%s] Found: %d ", CA2W(szIP).m_szBuffer, m_ResultList.GetItemCount());
	SetWindowText(Title);
	return 0;
}

void CFileMgrSearchDlg::OnBnClickedOk()
{
	CWnd*pButton = GetDlgItem(IDOK);
	CString Text;
	pButton->GetWindowText(Text);

	if (Text == L"Stop")
	{
		m_pHandler->Stop();
	}
	else
	{
		//Search
		UpdateData();
		if (m_TargetName.GetLength() == 0 || m_StartLocation.GetLength() == 0)
			return;
		//开始查找
		char szIP[128];
		USHORT uPort;
		m_pHandler->GetPeerName(szIP, uPort);
		Text.Format(L"[%s] Searching...", CA2W(szIP).m_szBuffer);
		SetWindowText(Text);

		pButton->SetWindowTextW(L"Stop");
		m_ResultList.DeleteAllItems();
		m_pHandler->Search((m_StartLocation + L"\n" + m_TargetName).GetBuffer());
	}
}


void CFileMgrSearchDlg::OnClose()
{
	if (m_pHandler)
	{
		m_pHandler->Disconnect();
	}
}


void CFileMgrSearchDlg::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO:  在此添加控件通知处理程序代码
	*pResult = 0;
	//
	CString FileName;
	if (!m_ResultList.GetSelectedCount())
		return;
	POSITION pos = m_ResultList.GetFirstSelectedItemPosition();
	int nItem = m_ResultList.GetNextSelectedItem(pos);

	FileName += m_ResultList.GetItemText(nItem, 1);
	FileName += "\\";
	FileName += m_ResultList.GetItemText(nItem, 0);

	HGLOBAL hGlobal = GlobalAlloc(GHND | GMEM_SHARE, sizeof(WCHAR)*(FileName.GetLength() + 1));
	void* Buffer = GlobalLock(hGlobal);

	wcscpy((PWCHAR)Buffer, FileName.GetBuffer());
	GlobalUnlock(hGlobal);

	OpenClipboard();
	EmptyClipboard();
	SetClipboardData(CF_UNICODETEXT, hGlobal);
	CloseClipboard();
}
