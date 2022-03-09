// FileSelectDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "HoldingHands.h"
#include "FileSelectDlg.h"
#include "afxdialogex.h"


// CFileSelectDlg 对话框

IMPLEMENT_DYNAMIC(CFileSelectDlg, CDialogEx)

CFileSelectDlg::CFileSelectDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFileSelectDlg::IDD, pParent)
	, m_FileList(_T(""))
{
}

CFileSelectDlg::~CFileSelectDlg()
{
}

void CFileSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MFCSHELLLIST1, m_List);
	DDX_Control(pDX, IDC_MFCSHELLTREE1, m_Tree);
	DDX_Control(pDX, IDC_EDIT1, m_SelFiles);
}


BEGIN_MESSAGE_MAP(CFileSelectDlg, CDialogEx)
	
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_MFCSHELLLIST1, &CFileSelectDlg::OnLvnItemchangedMfcshelllist1)
	ON_NOTIFY(LVN_DELETEALLITEMS, IDC_MFCSHELLLIST1, &CFileSelectDlg::OnLvnDeleteallitemsMfcshelllist1)
	ON_BN_CLICKED(IDOK, &CFileSelectDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CFileSelectDlg 消息处理程序



BOOL CFileSelectDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	
	m_List.DeleteAllItems();
	m_Tree.SetRelatedList(&m_List);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}


void CFileSelectDlg::OnLvnItemchangedMfcshelllist1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO:  在此添加控件通知处理程序代码
	*pResult = 0;
	CString Text;
	//被选择了
	if (!(pNMLV->uOldState &LVIS_SELECTED ) && (pNMLV->uNewState &LVIS_SELECTED))
	{
		m_MapFileList.SetAt(m_List.GetItemText(pNMLV->iItem, 0), 0);
		POSITION pos = m_MapFileList.GetStartPosition();
		while (pos)
		{
			CString Key; void* value;
			m_MapFileList.GetNextAssoc(pos, Key, value);
			Text += L"\"" + Key + L"\" ";
		}
		m_SelFiles.SetWindowTextW(Text);
	}
	//被取消了
	else if (!(pNMLV->uNewState &LVIS_SELECTED )&& (pNMLV->uOldState &LVIS_SELECTED))
	{
		m_MapFileList.RemoveKey(m_List.GetItemText(pNMLV->iItem, 0));
		POSITION pos = m_MapFileList.GetStartPosition();
		while (pos)
		{
			CString Key; void* value;
			m_MapFileList.GetNextAssoc(pos, Key, value);
			Text += L"\"" + Key + L"\"";
		}
		m_SelFiles.SetWindowTextW(Text);
	}
}


void CFileSelectDlg::OnLvnDeleteallitemsMfcshelllist1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO:  在此添加控件通知处理程序代码
	*pResult = 0;
	//在没有创建edit之前也会调用这个函数mmp.
	if (m_SelFiles.m_hWnd)
	{
		m_MapFileList.RemoveAll();
		m_SelFiles.SetWindowTextW(L"");
	}
}


void CFileSelectDlg::OnBnClickedOk()
{
	// TODO:  在此添加控件通知处理程序代码
	CString CurDir;
	if (m_List.GetCurrentFolder(CurDir))
	{
		m_FileList = CurDir;
		m_FileList += "\n";

		POSITION pos = m_MapFileList.GetStartPosition();
		while (pos)
		{
			CString Key; void* value;
			m_MapFileList.GetNextAssoc(pos, Key, value);
			m_FileList += Key;
			m_FileList +="\n";
		}
		MessageBox(m_FileList);
	}
	else
		m_FileList = L"";		//没有路径
	CDialogEx::OnOK();
}
