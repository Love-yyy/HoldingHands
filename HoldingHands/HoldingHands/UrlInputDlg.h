#pragma once


// CUrlInputDlg 对话框

class CUrlInputDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CUrlInputDlg)

public:
	CUrlInputDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CUrlInputDlg();

// 对话框数据
	enum { IDD = IDD_URL_INPUT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_Url;
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
};
