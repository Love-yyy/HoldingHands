#pragma once


// CFileMgrInputNameDlg 对话框

class CFileMgrInputNameDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFileMgrInputNameDlg)

public:
	CFileMgrInputNameDlg(CString FileName,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CFileMgrInputNameDlg();

// 对话框数据
	enum { IDD = IDD_FILE_NAME_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_FileName;
	afx_msg void OnBnClickedOk();
};
