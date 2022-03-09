// UrlInputDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "HoldingHands.h"
#include "UrlInputDlg.h"
#include "afxdialogex.h"


// CUrlInputDlg 对话框

IMPLEMENT_DYNAMIC(CUrlInputDlg, CDialogEx)

CUrlInputDlg::CUrlInputDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CUrlInputDlg::IDD, pParent)
	, m_Url(_T(""))
{

}

CUrlInputDlg::~CUrlInputDlg()
{
}

void CUrlInputDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_Url);
	DDV_MaxChars(pDX, m_Url, 4095);
}


BEGIN_MESSAGE_MAP(CUrlInputDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CUrlInputDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CUrlInputDlg 消息处理程序


void CUrlInputDlg::OnBnClickedOk()
{
	UpdateData();
	if (!m_Url.GetLength())
		return;
	CDialogEx::OnOK();
}


BOOL CUrlInputDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	GetDlgItem(IDC_EDIT1)->SetFocus();
	return FALSE;  // return TRUE unless you set the focus to a control
}
