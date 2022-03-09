#pragma once
#include "afxcmn.h"
#include "resource.h"
#define WM_MNDD_FILEINFO			(WM_USER+701)
#define WM_MNDD_DOWNLOAD_RESULT		(WM_USER+702)
// CMiniDownloadDlg 对话框

class CMiniDownloadSrv;

class CMiniDownloadDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMiniDownloadDlg)

public:
	CMiniDownloadDlg(CMiniDownloadSrv* pHandler, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMiniDownloadDlg();

// 对话框数据
	enum { IDD = IDD_MNDD_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CProgressCtrl m_Progress;
	CMiniDownloadSrv*	m_pHandler;
	ULONGLONG			m_ullTotalSize;
	ULONGLONG			m_ullFinishedSize;
	CString				m_IP;
	afx_msg LRESULT OnGetFileInfoReply(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDownloadReply(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
};
