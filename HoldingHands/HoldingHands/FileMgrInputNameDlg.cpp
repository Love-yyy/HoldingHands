// FileMgrInputNameDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "HoldingHands.h"
#include "FileMgrInputNameDlg.h"
#include "afxdialogex.h"


// CFileMgrInputNameDlg 对话框

IMPLEMENT_DYNAMIC(CFileMgrInputNameDlg, CDialogEx)

CFileMgrInputNameDlg::CFileMgrInputNameDlg(CString FileName, CWnd* pParent /*=NULL*/)
	: CDialogEx(CFileMgrInputNameDlg::IDD, pParent)
	, m_FileName(FileName)
{

}

CFileMgrInputNameDlg::~CFileMgrInputNameDlg()
{
}

void CFileMgrInputNameDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_FileName);
}


BEGIN_MESSAGE_MAP(CFileMgrInputNameDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CFileMgrInputNameDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CFileMgrInputNameDlg 消息处理程序


void CFileMgrInputNameDlg::OnBnClickedOk()
{
	UpdateData(TRUE);
	if (m_FileName.GetLength() == 0)
		return;
	CDialogEx::OnOK();
}
