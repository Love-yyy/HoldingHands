#pragma once
#include "afxwin.h"

class CCmdSrv;

class CCmdWnd :
	public CFrameWnd
{
public:
	CCmdWnd(CCmdSrv*pHandler);
	~CCmdWnd();

	CCmdSrv *	m_pHandler;
	CFont m_Font;
	CEdit m_CmdShow;
	CEdit m_Cmd;

	CList<CString>	m_Commands;
	POSITION		m_LastCommand;
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

