#pragma once
#include "afxwin.h"
#include "resource.h"
class CRemoteDesktopSrv;

#define WM_REMOTE_DESKTOP_ERROR			(WM_USER + 69)
#define WM_REMOTE_DESKTOP_SIZE			(WM_USER + 71)
#define WM_REMOTE_DESKTOP_DRAW			(WM_USER + 70)

#define SCROLL_UINT		5.0

class CRemoteDesktopWnd :
	public CFrameWnd
{
public:
	CRemoteDesktopSrv*m_pHandler;
	DWORD	m_dwLastTime;
	DWORD	m_dwOldFps;
	DWORD	m_dwFps;

	DWORD	m_dwDeskWidth;		//客户区最大尺寸
	DWORD	m_dwDeskHeight;
							
	DWORD	m_dwMaxHeight;		//窗口最大尺寸
	DWORD	m_dwMaxWidth;

	HDC		m_hDC;

	POINT	m_OrgPt;			//滚动条位置.

	BOOL	m_bStretch;

	BOOL	m_bCaptureMouse;
	BOOL	m_bCaptureTransparentWnd;

	BOOL	m_bCtrlMouse;
	BOOL	m_bCtrlKeyboard;


	CMenu	m_Menu;
	CRemoteDesktopWnd(CRemoteDesktopSrv*pHandler);
	~CRemoteDesktopWnd();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnClose();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDisplayStretch();
	afx_msg void OnDisplayTile();
	afx_msg void OnCaptureMouse();
	afx_msg void OnControlKeyboard();
	afx_msg void OnControlMouse();
	afx_msg void OnCaptureTransparentwindow();
	afx_msg void OnUpdateControlMouse(CCmdUI *pCmdUI);
	afx_msg void OnUpdateControlKeyboard(CCmdUI *pCmdUI);
	afx_msg void OnUpdateCaptureMouse(CCmdUI *pCmdUI);
	afx_msg void OnUpdateCaptureTransparentwindow(CCmdUI *pCmdUI);

	LRESULT	OnDraw(WPARAM wParam, LPARAM lParam);
	LRESULT OnError(WPARAM wParam, LPARAM lParam);
	LRESULT OnDesktopSize(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);

	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnMaxfps10();
	afx_msg void OnMaxfps20();
	afx_msg void OnMaxfps30();
	afx_msg void OnMaxfpsNolimit();
};

