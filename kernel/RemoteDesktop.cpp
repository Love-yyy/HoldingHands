#include "RemoteDesktop.h"
#include "IOCPClient.h"
#include "x264EncoderLoad.h"

#include <WinUser.h>

CRemoteDesktop::CRemoteDesktop(DWORD dwIdentity):
CEventHandler(dwIdentity)
{
	m_dwLastTime = 0;
	m_dwFrameSize = 0;
	m_FrameBuffer = 0;
	m_dwMaxFps = 20;
}


CRemoteDesktop::~CRemoteDesktop()
{
}

void CRemoteDesktop::OnClose()
{

}
void CRemoteDesktop::OnConnect()
{
	WCHAR szError[0x1000];
	WCHAR szModulePath[0x1000];
	//
	GetModuleFileNameW(GetModuleHandle(NULL),szModulePath,0x1000);
	//
	WCHAR*p = szModulePath + wcslen(szModulePath) - 1;
	while(p>=szModulePath)
	{
		if(*p == '\\' || *p == '/')
			break;
		p--;
	}
	if(p < szModulePath)
	{
		//invalid path;
		wcscpy(szError,L"Get Module File Name Failed!");
		Send(REMOTEDESKTOP_ERROR,(char*)szError,sizeof(WCHAR) * (wcslen(szError) + 1));
		
		Disconnect();
		return;
	}
	p[0] = NULL;
	wcscat(szModulePath,L"\\modules\\");
	
	if(x264EncoderLoad(szModulePath,szError) == false)
	{
		//load x264 module failed!
		Send(REMOTEDESKTOP_ERROR,(char*)szError,sizeof(WCHAR) * (wcslen(szError) + 1));
		
		Disconnect();
		return;
	}
}

void CRemoteDesktop::OnReadPartial(WORD Event, DWORD Total, DWORD Read, char*Buffer)
{

}
void CRemoteDesktop::OnReadComplete(WORD Event, DWORD Total, DWORD Read, char*Buffer)
{
	switch (Event)
	{
	case REMOTEDESKTOP_NEXT_FRAME:
		OnNextFrame(Buffer);
		break;
	case REMOTEDESKTOP_GETSIZE:
		OnGetSize();
		break;
	case REMOTEDESKTOP_CTRL:
		//_OnControl((CtrlParam*)Buffer);
		OnControl((CtrlParam*)Buffer);
		break;
	case REMOTEDESKTOP_SETMAXFPS:
		OnSetMaxFps(*(DWORD*)Buffer);
		break;
	default:
		break;
	}
}

void CRemoteDesktop::OnGetSize()
{
	WCHAR szError[] = L"desktop grab init failed!";

	if (FALSE == m_grab.GrabInit())
	{
		Send(REMOTEDESKTOP_ERROR, (char*)szError, (sizeof(WCHAR) * (wcslen(szError) + 1)));
		Disconnect();
		return;
	}
	//成功,发送大小.
	DWORD buff[2];
	m_grab.GetDesktopSize(buff, buff + 1);
	Send(REMOTEDESKTOP_DESKSIZE, (char*)buff, sizeof(DWORD) * 2);
	//立刻开始编码
	m_dwLastTime = GetTickCount();
	if (!m_grab.GetFrame(&m_FrameBuffer, &m_dwFrameSize))
	{
		Send(REMOTEDESKTOP_ERROR, (char*)szError, (sizeof(WCHAR) * (wcslen(szError) + 1)));
		Disconnect();
	}
}
void CRemoteDesktop::OnNextFrame(char*flag)
{

	WCHAR szError[] = L"desktop grab failed!";
	DWORD dwUsedTime;
	//发送上一帧.
	Send(REMOTEDESKTOP_FRAME, m_FrameBuffer, m_dwFrameSize);
	//防止太快.
	dwUsedTime = (GetTickCount() - m_dwLastTime) - 10;
	if (dwUsedTime < (1000 / m_dwMaxFps))
		Sleep((1000 / m_dwMaxFps) - dwUsedTime);

	m_dwLastTime = GetTickCount();
	if (!m_grab.GetFrame(&m_FrameBuffer, &m_dwFrameSize, flag[0], flag[1]))
	{
		Send(REMOTEDESKTOP_ERROR, (char*)szError, (sizeof(WCHAR) * (wcslen(szError) + 1)));
		Disconnect();
	}
}
void CRemoteDesktop::OnSetMaxFps(DWORD dwMaxFps)
{
	m_dwMaxFps = dwMaxFps;
}


void CRemoteDesktop::OnControl(CtrlParam*pParam)
{
	switch (pParam->dwType)
	{
	case WM_KEYDOWN:
		keybd_event(pParam->Param.VkCode, 0, 0, 0);
		break;
	case WM_KEYUP:
		keybd_event(pParam->Param.VkCode, 0, KEYEVENTF_KEYUP, 0);
		break;
		//鼠标移动
	case WM_MOUSEMOVE:
		mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, LOWORD(pParam->Param.dwCoor), HIWORD(pParam->Param.dwCoor), 0, 0);
		break;
		//左键操作
	case WM_LBUTTONDOWN:
		mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN, LOWORD(pParam->Param.dwCoor), HIWORD(pParam->Param.dwCoor), 0, 0);
		break;
	case WM_LBUTTONUP:
		mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP, LOWORD(pParam->Param.dwCoor), HIWORD(pParam->Param.dwCoor), 0, 0);
		break;
	case WM_LBUTTONDBLCLK:
		printf("RemoteDesktop::OnControl::LBUTTONDBCLICK");
		mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN|MOUSEEVENTF_LEFTUP, LOWORD(pParam->Param.dwCoor), HIWORD(pParam->Param.dwCoor), 0, 0);
		Sleep(50);
		mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, LOWORD(pParam->Param.dwCoor), HIWORD(pParam->Param.dwCoor), 0, 0);
		break;
		//右键操作
	case WM_RBUTTONDOWN:
		mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTDOWN, LOWORD(pParam->Param.dwCoor), HIWORD(pParam->Param.dwCoor), 0, 0);
		break;
	case WM_RBUTTONUP:
		mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTUP, LOWORD(pParam->Param.dwCoor), HIWORD(pParam->Param.dwCoor), 0, 0);
		break;
	case WM_RBUTTONDBLCLK:
		mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTDOWN|MOUSEEVENTF_RIGHTUP, LOWORD(pParam->Param.dwCoor), HIWORD(pParam->Param.dwCoor), 0, 0);
		Sleep(50);
		mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, LOWORD(pParam->Param.dwCoor), HIWORD(pParam->Param.dwCoor), 0, 0);
		break;
		//中键操作
	case WM_MBUTTONDOWN:
		mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MIDDLEDOWN,0, 0, 0, 0);
		break;
	case WM_MBUTTONUP:
		mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
		break;
	case WM_MBUTTONDBLCLK:
		mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MIDDLEDOWN|MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
		Sleep(50);
		mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MIDDLEDOWN|MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
		break;
		//
	case 0x020A:		//mouse wheel
		mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_WHEEL, 0, 0, pParam->dwExtraData, 0);
		break;
	default:
		break;
	}
}