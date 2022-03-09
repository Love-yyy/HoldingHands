#include "stdafx.h"
#include "RemoteDesktopWnd.h"
#include "RemoteDesktopSrv.h"

#pragma comment(lib,"HHRDKbHook.lib")

DWORD			dwWndCount = 0;			//窗口计数
HHOOK			hKbHook = NULL;
extern"C"__declspec(dllimport)	HWND	hTopWindow;						//顶层窗口
extern"C"__declspec(dllimport) LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);	//钩子函数

CRemoteDesktopWnd::CRemoteDesktopWnd(CRemoteDesktopSrv*pHandler)
{
	m_bStretch = FALSE;
	m_bCaptureMouse = FALSE;
	m_bCaptureTransparentWnd = FALSE;

	m_bCtrlMouse = FALSE;
	m_bCtrlKeyboard = FALSE;

	m_pHandler = pHandler;

	m_dwDeskWidth = 0;
	m_dwDeskHeight = 0;
	m_dwMaxHeight = 0;
	m_dwMaxWidth = 0;

	m_dwLastTime = 0;
	m_dwFps = 0;
	m_dwOldFps = 0;

	m_hDC = NULL;
	//
	if (!dwWndCount)
	{

		hKbHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(L"HHRDKbHook.dll"), NULL);
		dwWndCount++;
	}
}


CRemoteDesktopWnd::~CRemoteDesktopWnd()
{
	//构造函数与析构函数是在UI线程,应该不用担心线程安全问题.
	dwWndCount--;
	if (!dwWndCount)
	{
		UnhookWindowsHookEx(hKbHook);
		hKbHook = NULL;
	}
}
BEGIN_MESSAGE_MAP(CRemoteDesktopWnd, CFrameWnd)
	ON_WM_CLOSE()
	ON_WM_CREATE()
	ON_COMMAND(ID_DISPLAY_STRETCH, &CRemoteDesktopWnd::OnDisplayStretch)
	ON_COMMAND(ID_DISPLAY_TILE, &CRemoteDesktopWnd::OnDisplayTile)
	ON_COMMAND(ID_CAPTURE_MOUSE, &CRemoteDesktopWnd::OnCaptureMouse)
	ON_COMMAND(ID_CONTROL_KEYBOARD, &CRemoteDesktopWnd::OnControlKeyboard)
	ON_COMMAND(ID_CONTROL_MOUSE, &CRemoteDesktopWnd::OnControlMouse)
	ON_COMMAND(ID_CAPTURE_TRANSPARENTWINDOW, &CRemoteDesktopWnd::OnCaptureTransparentwindow)
	ON_UPDATE_COMMAND_UI(ID_CONTROL_MOUSE, &CRemoteDesktopWnd::OnUpdateControlMouse)
	ON_UPDATE_COMMAND_UI(ID_CONTROL_KEYBOARD, &CRemoteDesktopWnd::OnUpdateControlKeyboard)
	ON_UPDATE_COMMAND_UI(ID_CAPTURE_MOUSE, &CRemoteDesktopWnd::OnUpdateCaptureMouse)
	ON_UPDATE_COMMAND_UI(ID_CAPTURE_TRANSPARENTWINDOW, &CRemoteDesktopWnd::OnUpdateCaptureTransparentwindow)
	ON_MESSAGE(WM_REMOTE_DESKTOP_ERROR,OnError)
	ON_MESSAGE(WM_REMOTE_DESKTOP_DRAW, OnDraw)
	ON_MESSAGE(WM_REMOTE_DESKTOP_SIZE,OnDesktopSize)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_MAXFPS_10, &CRemoteDesktopWnd::OnMaxfps10)
	ON_COMMAND(ID_MAXFPS_20, &CRemoteDesktopWnd::OnMaxfps20)
	ON_COMMAND(ID_MAXFPS_30, &CRemoteDesktopWnd::OnMaxfps30)
	ON_COMMAND(ID_MAXFPS_NOLIMIT, &CRemoteDesktopWnd::OnMaxfpsNolimit)
END_MESSAGE_MAP()


void CRemoteDesktopWnd::OnClose()
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	if (m_pHandler)
	{
		m_pHandler->Disconnect();
	}
}


int CRemoteDesktopWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	m_Menu.LoadMenuW(IDR_RD_MENU);
	SetMenu(&m_Menu);
	//默认是平铺.
	CMenu*pMenu = m_Menu.GetSubMenu(0);
	pMenu->CheckMenuRadioItem(ID_DISPLAY_STRETCH, ID_DISPLAY_TILE, ID_DISPLAY_TILE, MF_BYCOMMAND);
	
	// TODO:  在此添加您专用的创建代码
	char szIP[128];
	USHORT uPort;
	m_pHandler->GetPeerName(szIP, uPort);

	CString Title;
	Title.Format(L"[%s] RemoteDesktop", CA2W(szIP).m_szBuffer);
	SetWindowText(Title);

	m_hDC = ::GetDC(m_hWnd);
	SetStretchBltMode(m_hDC,HALFTONE);
	SetBkMode(m_hDC,TRANSPARENT);
	SetTextColor(m_hDC,0x000033ff);

	pMenu->GetSubMenu(2)->CheckMenuRadioItem(ID_MAXFPS_10, ID_MAXFPS_NOLIMIT, ID_MAXFPS_20, MF_BYCOMMAND);
	return 0;
}


void CRemoteDesktopWnd::OnDisplayStretch()
{
	// TODO:  在此添加命令处理程序代码
	m_bStretch = TRUE;
	CMenu*pMenu = m_Menu.GetSubMenu(0);
	EnableScrollBarCtrl(SB_VERT, 0);
	EnableScrollBarCtrl(SB_HORZ, 0);
	pMenu->CheckMenuRadioItem(ID_DISPLAY_STRETCH, ID_DISPLAY_TILE, ID_DISPLAY_STRETCH, MF_BYCOMMAND);
}


void CRemoteDesktopWnd::OnDisplayTile()
{
	// TODO:  在此添加命令处理程序代码
	m_bStretch = FALSE;
	CMenu*pMenu = m_Menu.GetSubMenu(0);
	EnableScrollBarCtrl(SB_VERT, 1);
	EnableScrollBarCtrl(SB_HORZ, 1);
	//
	//设置滚动条范围
	CRect rect;
	GetClientRect(rect);
	SetScrollRange(SB_HORZ, 0, (m_dwDeskWidth - rect.right) / SCROLL_UINT);
	SetScrollRange(SB_VERT, 0, (m_dwDeskHeight - rect.bottom) / SCROLL_UINT);
	m_OrgPt.x = m_OrgPt.y = 0;
	SetScrollPos(SB_VERT, m_OrgPt.y);
	SetScrollPos(SB_HORZ, m_OrgPt.x);

	pMenu->CheckMenuRadioItem(ID_DISPLAY_STRETCH, ID_DISPLAY_TILE, ID_DISPLAY_TILE, MF_BYCOMMAND);
}


void CRemoteDesktopWnd::OnCaptureMouse()
{
	m_bCaptureMouse = !m_bCaptureMouse;
}

void CRemoteDesktopWnd::OnCaptureTransparentwindow()
{
	m_bCaptureTransparentWnd = !m_bCaptureTransparentWnd;
}


void CRemoteDesktopWnd::OnControlKeyboard()
{
	m_bCtrlKeyboard = !m_bCtrlKeyboard;
}


void CRemoteDesktopWnd::OnControlMouse()
{
	m_bCtrlMouse = !m_bCtrlMouse;
}





void CRemoteDesktopWnd::OnUpdateControlMouse(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bCtrlMouse);
}


void CRemoteDesktopWnd::OnUpdateControlKeyboard(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bCtrlKeyboard);
}


void CRemoteDesktopWnd::OnUpdateCaptureMouse(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bCaptureMouse);
}


void CRemoteDesktopWnd::OnUpdateCaptureTransparentwindow(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bCaptureTransparentWnd);
}

LRESULT CRemoteDesktopWnd::OnError(WPARAM wParam, LPARAM lParam)
{
	WCHAR*Tips = (WCHAR*)wParam;
	MessageBox(Tips, L"Error");
	return 0;
}

LRESULT CRemoteDesktopWnd::OnDesktopSize(WPARAM wParam, LPARAM lParam)
{
	m_dwDeskWidth = wParam;
	m_dwDeskHeight = lParam;
	
	CRect WndRect;
	CRect ClientRect;
	//
	GetWindowRect(WndRect);
	//
	GetClientRect(ClientRect);

	if (!m_bStretch)
	{
		//设置滚动条范围
		SetScrollRange(SB_HORZ, 0, (m_dwDeskWidth - ClientRect.right) / SCROLL_UINT);
		SetScrollRange(SB_VERT, 0, (m_dwDeskHeight - ClientRect.bottom) / SCROLL_UINT);
		m_OrgPt.x = m_OrgPt.y = 0;
		SetScrollPos(SB_VERT, m_OrgPt.y);
		SetScrollPos(SB_HORZ, m_OrgPt.x);
	}
	
	ClientToScreen(ClientRect);
	//
	m_dwMaxHeight = m_dwDeskHeight + ClientRect.top - WndRect.top + WndRect.bottom - ClientRect.bottom;
	m_dwMaxWidth = m_dwDeskWidth + ClientRect.left - WndRect.left + WndRect.right - ClientRect.right;
	return 0;
}
LRESULT CRemoteDesktopWnd::OnDraw(WPARAM wParam, LPARAM lParam)
{

	HDC hMemDC = (HDC)wParam;
	BITMAP*pBitmap = (BITMAP*)lParam;
	CString Fps;
	
	if (m_bStretch)
	{
		RECT rect = { 0 };
		GetClientRect(&rect);
		::StretchBlt(m_hDC, 0, 0, rect.right, rect.bottom, hMemDC, 0, 0, pBitmap->bmWidth, pBitmap->bmHeight, SRCCOPY);
	}
	else
		::BitBlt(m_hDC, -m_OrgPt.x*SCROLL_UINT, -m_OrgPt.y*SCROLL_UINT, m_dwDeskWidth, m_dwDeskHeight, hMemDC, 0, 0, SRCCOPY);
	m_dwFps++;

	
	Fps.Format(L"Fps: %d", m_dwOldFps);
	
	TextOutW(m_hDC,10, 10, Fps,Fps.GetLength());

	if ((GetTickCount() - m_dwLastTime) >= 1000)
	{
		m_dwOldFps = m_dwFps;
		//刷新
		m_dwLastTime = GetTickCount();
		m_dwFps = 0;
	}
	return 0;
}



BOOL CRemoteDesktopWnd::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	// TODO:  在此添加专用代码和/或调用基类
	if (!m_bCtrlKeyboard && !m_bCtrlMouse)
		return CFrameWnd::OnWndMsg(message, wParam, lParam, pResult);
	
	CtrlParam2* pParam = NULL;
	DWORD dwParamSize = sizeof(DWORD) + sizeof(INPUT);
	if (message >= WM_MOUSEMOVE && message <= WM_MOUSEWHEEL && m_bCtrlMouse)
	{
		DWORD dwX, dwY;
		dwX = LOWORD(lParam);
		dwY = HIWORD(lParam);
		if (m_bStretch)
		{
			RECT rect;
			GetClientRect(&rect);
			dwX = dwX * 65535.0 / rect.right;
			dwY = dwY * 65535.0 / rect.bottom;
		}
		else
		{
			dwX += m_OrgPt.x * SCROLL_UINT;
			dwY += m_OrgPt.y * SCROLL_UINT;

			dwX = dwX * 65535.0 / m_dwDeskWidth;
			dwY = dwY * 65535.0 / m_dwDeskHeight;
		}
		

		if (message == WM_LBUTTONDBLCLK || message == WM_RBUTTONDBLCLK || WM_MBUTTONDBLCLK) 
		{
			dwParamSize += sizeof(INPUT);
		}

		pParam = (CtrlParam2*)malloc(dwParamSize);
		memset(pParam, 0, dwParamSize);
		pParam->m_dwCount = 1;
		
		pParam->m_inputs[0].type = INPUT_MOUSE;
		pParam->m_inputs[0].mi.dwFlags |= MOUSEEVENTF_ABSOLUTE;
		pParam->m_inputs[0].mi.dx = dwX;
		pParam->m_inputs[0].mi.dy = dwY;
		switch (message)
		{
			case WM_MOUSEMOVE:
				pParam->m_inputs[0].mi.dwFlags |= MOUSEEVENTF_MOVE ;
				break;
			case WM_LBUTTONDOWN:
				pParam->m_inputs[0].mi.dwFlags |= MOUSEEVENTF_LEFTDOWN;
				break;
			case WM_LBUTTONUP:
				pParam->m_inputs[0].mi.dwFlags |= MOUSEEVENTF_LEFTUP ;
				break;
			case WM_LBUTTONDBLCLK:
				pParam->m_dwCount = 2;
				pParam->m_inputs[0].mi.dwFlags |= MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP ;
				memcpy(&pParam->m_inputs[1], &pParam->m_inputs[0], sizeof(INPUT));
				break;
			case WM_RBUTTONDOWN:
				pParam->m_inputs[0].mi.dwFlags |= MOUSEEVENTF_RIGHTDOWN ;
				break;
			case WM_RBUTTONUP:
				pParam->m_inputs[0].mi.dwFlags |= MOUSEEVENTF_RIGHTUP;
				break;
			case WM_RBUTTONDBLCLK:
				pParam->m_dwCount = 2;
				pParam->m_inputs[0].mi.dwFlags |= MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP;
				memcpy(&pParam->m_inputs[1], &pParam->m_inputs[0], sizeof(INPUT));
				break;
			case WM_MBUTTONDOWN:
				pParam->m_inputs[0].mi.dwFlags |= MOUSEEVENTF_MIDDLEDOWN;
				break;
			case WM_MBUTTONUP:
				pParam->m_inputs[0].mi.dwFlags |= MOUSEEVENTF_MIDDLEUP;
				break;
			case WM_MBUTTONDBLCLK:
				pParam->m_dwCount = 2;
				pParam->m_inputs[0].mi.dwFlags |= MOUSEEVENTF_MIDDLEDOWN | MOUSEEVENTF_MIDDLEUP;
				memcpy(&pParam->m_inputs[1], &pParam->m_inputs[0], sizeof(INPUT));
				break;
			case WM_MOUSEWHEEL:
				pParam->m_inputs[0].mi.dwFlags |= MOUSEEVENTF_WHEEL;
				pParam->m_inputs[0].mi.mouseData = (SHORT)HIWORD(wParam);
				break;
		}
		m_pHandler->Control2(pParam);
		free(pParam);
		return TRUE;
	}
	if ((message == WM_KEYDOWN || message == WM_KEYUP) && m_bCtrlKeyboard)
	{
		pParam = (CtrlParam2*)malloc(dwParamSize);
		memset(pParam, 0, dwParamSize);
		pParam->m_dwCount = 1;
		pParam->m_inputs[0].type = INPUT_KEYBOARD;
		pParam->m_inputs[0].ki.wVk = wParam;
		if (message == WM_KEYUP)
			pParam->m_inputs[0].ki.dwFlags = KEYEVENTF_KEYUP;

		m_pHandler->Control2(pParam);
		free(pParam);
		return TRUE;
	}
	return CFrameWnd::OnWndMsg(message, wParam, lParam, pResult);
}


void CRemoteDesktopWnd::OnSize(UINT nType, int cx, int cy)
{
	// TODO:  在此处添加消息处理程序代码
	if (!m_bStretch && m_dwMaxHeight != 0 &&m_dwMaxWidth !=0 )
	{
		//限制大小.
		if (cx > m_dwDeskWidth || cy > m_dwDeskHeight)
		{
			CRect rect;
			GetWindowRect(rect);
			if (cx > m_dwDeskWidth)
				rect.right = rect.left + m_dwMaxWidth;
			if (cy > m_dwDeskHeight)
				rect.bottom = rect.top + m_dwMaxHeight;
			MoveWindow(rect, 1);
			return CFrameWnd::OnSize(nType, cx, cy);
		}
		//计算滚动条.
		SetScrollRange(SB_VERT, 0, (m_dwDeskHeight - cy) / SCROLL_UINT);
		SetScrollRange(SB_HORZ, 0, (m_dwDeskWidth - cx) / SCROLL_UINT);
		//限制水平位置,不能超过最大值
		m_OrgPt.x = min(m_OrgPt.x, (m_dwDeskWidth - cx) / SCROLL_UINT);
		m_OrgPt.y = min(m_OrgPt.y, (m_dwDeskHeight - cy) / SCROLL_UINT);
		//重新设置Pos
		SetScrollPos(SB_HORZ, m_OrgPt.x);
		SetScrollPos(SB_VERT, m_OrgPt.y);
	}
	CFrameWnd::OnSize(nType, cx, cy);
}


BOOL CRemoteDesktopWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO:  在此添加专用代码和/或调用基类
	cs.style |= WS_VSCROLL;
	cs.style |= WS_HSCROLL;

	if (CFrameWnd::PreCreateWindow(cs))
	{
		static BOOL bRegistered = FALSE;

		if (!bRegistered)
		{
			WNDCLASS wndclass = { 0 };
			GetClassInfo(AfxGetInstanceHandle(), cs.lpszClass, &wndclass);
			//取消水平大小和竖直大小变化时的重绘.没用OnPaint,会白屏.
			wndclass.style &= ~(CS_HREDRAW |CS_VREDRAW );
			wndclass.lpszClassName = L"RemoteDesktopWndClass";

			bRegistered = AfxRegisterClass(&wndclass);
		}
		if (!bRegistered)
			return FALSE;
		cs.lpszClass = L"RemoteDesktopWndClass";
		return TRUE;
	}
	return FALSE;
}


void CRemoteDesktopWnd::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	if (m_bStretch == FALSE)
	{
		lpMMI->ptMaxSize.x = m_dwMaxWidth;
		lpMMI->ptMaxSize.y = m_dwMaxHeight;
	}

	CFrameWnd::OnGetMinMaxInfo(lpMMI);
}


void CRemoteDesktopWnd::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	SCROLLINFO si = { 0 };
	CRect rect;
	GetClientRect(rect);
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;

	GetScrollInfo(SB_HORZ, &si);
	
	switch (nSBCode)
	{
	case SB_LINEUP:
		m_OrgPt.x--;
		break;
	case SB_LINEDOWN:
		m_OrgPt.x++;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		m_OrgPt.x = si.nTrackPos;
		break;
	default:
		break;
	}
	//
	m_OrgPt.x = min((m_dwDeskWidth - rect.right) / SCROLL_UINT, m_OrgPt.x);
	m_OrgPt.x = max(0, m_OrgPt.x);
	SetScrollPos(SB_HORZ,m_OrgPt.x);
	CFrameWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CRemoteDesktopWnd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	SCROLLINFO si = { 0 };
	CRect rect;
	GetClientRect(rect);
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;

	GetScrollInfo(SB_VERT, &si);

	switch (nSBCode)
	{
	case SB_LINEUP:
		m_OrgPt.y--;
		break;
	case SB_LINEDOWN:
		m_OrgPt.y++;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		m_OrgPt.y = si.nTrackPos;
		break;
	default:
		break;
	}
	//
	m_OrgPt.y = min((m_dwDeskHeight - rect.bottom) / SCROLL_UINT, m_OrgPt.y);
	m_OrgPt.y = max(0, m_OrgPt.y);
	SetScrollPos(SB_VERT, m_OrgPt.y);

	CFrameWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CRemoteDesktopWnd::OnSetFocus(CWnd* pOldWnd)
{
	CFrameWnd::OnSetFocus(pOldWnd);

	// TODO:  在此处添加消息处理程序代码
	hTopWindow = m_hWnd;
}


void CRemoteDesktopWnd::OnMaxfps10()
{
	m_pHandler->SetMaxFps(10);
	GetMenu()->GetSubMenu(0)->GetSubMenu(2)->CheckMenuRadioItem(ID_MAXFPS_10, ID_MAXFPS_NOLIMIT, ID_MAXFPS_10, MF_BYCOMMAND);
}


void CRemoteDesktopWnd::OnMaxfps20()
{
	m_pHandler->SetMaxFps(20);
	GetMenu()->GetSubMenu(0)->GetSubMenu(2)->CheckMenuRadioItem(ID_MAXFPS_10, ID_MAXFPS_NOLIMIT, ID_MAXFPS_20, MF_BYCOMMAND);
}


void CRemoteDesktopWnd::OnMaxfps30()
{
	m_pHandler->SetMaxFps(30);
	GetMenu()->GetSubMenu(0)->GetSubMenu(2)->CheckMenuRadioItem(ID_MAXFPS_10, ID_MAXFPS_NOLIMIT, ID_MAXFPS_30, MF_BYCOMMAND);
}


void CRemoteDesktopWnd::OnMaxfpsNolimit()
{
	m_pHandler->SetMaxFps(-1);
	GetMenu()->GetSubMenu(0)->GetSubMenu(2)->CheckMenuRadioItem(ID_MAXFPS_10, ID_MAXFPS_NOLIMIT, ID_MAXFPS_NOLIMIT, MF_BYCOMMAND);
}
