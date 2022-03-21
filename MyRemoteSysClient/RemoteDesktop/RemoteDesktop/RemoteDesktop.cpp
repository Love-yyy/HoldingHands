#include "RemoteDesktop.h"
#include "IOCPClient.h"

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
		OnControl((CtrlParam2*)Buffer);
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


void CRemoteDesktop::OnControl(CtrlParam2*pParam)
{
	UINT uResult = SendInput(pParam->m_dwCount, pParam->m_inputs, sizeof(INPUT));
}