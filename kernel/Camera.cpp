#include "Camera.h"

#define STRSAFE_NO_DEPRECATE

CCamera::CCamera(DWORD dwIdentity) :
CEventHandler(dwIdentity)
{
	m_bStop = FALSE;
	m_hWorkThread = NULL;
}


CCamera::~CCamera()
{
	//mmmp ,UnbindHandler, Send will Failed! in OnStop();
	//OnStop();
}

void CCamera::OnConnect()
{
	//Load x264 Module;
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
		Send(CAMERA_ERROR,(char*)szError,sizeof(WCHAR) * (wcslen(szError) + 1));
		
		Disconnect();
		return;
	}
	p[0] = NULL;
	wcscat(szModulePath,L"\\modules\\");
	//
	if(x264EncoderLoad(szModulePath,szError) == false)
	{
		//x264 load failed!
		Send(CAMERA_ERROR,(char*)szError,sizeof(WCHAR) * (wcslen(szError) + 1));
		Disconnect();
		return;
	}
	//Load x264.dll success!
	list<CCameraGrab::DeviceInfo> &list = m_Grab.GetDeviceList();
	wstring sList;

	for (std::list<CCameraGrab::DeviceInfo>::iterator Device = list.begin(); Device != list.end(); Device++)
	{
		sList += Device->szName;
		sList += L"\t";
		for (map<DWORD,DWORD>::iterator Size = Device->VideoSize.begin(); Size != Device->VideoSize.end(); Size++)
		{
			WCHAR szVideoSize[128] = { 0 };
			wsprintfW(szVideoSize, L"%u x %u,", Size->first, Size->second);
			sList += szVideoSize;
		}
		sList += L"\n";
	}
	Send(CAMERA_DEVICELIST, (char*)sList.c_str(), sizeof(WCHAR)*(sList.length() + 1));
}

void CCamera::OnClose()
{
	OnStop();
	//一定要停止捕捉,否则线程继续发送会崩掉.
}

void CCamera::OnStart(int idx,DWORD dwWidth,DWORD dwHeight)
{
	DWORD dwResponse[3] = { 0 };
	dwResponse[0] = m_Grab.GrabberInit(idx, dwWidth,dwHeight);
	if (dwResponse[0])
	{
		dwResponse[1] = dwWidth;
		dwResponse[2] = dwHeight;
	}
	Send(CAMERA_VIDEOSIZE, (char*)dwResponse, sizeof(dwResponse));
	//
	m_Grab.StopGrab();						//停止捕捉

	//清理未释放的线程资源.
	InterlockedExchange(&m_bStop, TRUE);	//停止发送
	//关闭上一次未释放的资源.
	if (m_hWorkThread)
	{
		WaitForSingleObject(m_hWorkThread, INFINITE);		//等待线程退出
		CloseHandle(m_hWorkThread);
		m_hWorkThread = NULL;
	}

	//取消停止标记.
	InterlockedExchange(&m_bStop, FALSE);
	if (TRUE == m_Grab.StartGrab())
	{
		//创建线程推流.
		m_hWorkThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)WorkThread, (LPVOID)this, 0, 0);
	}
}

void CCamera::WorkThread(CCamera*pThis)
{
	BOOL bStop = FALSE;

	while (!InterlockedExchange(&pThis->m_bStop, FALSE))
	{
		char*Buffer = NULL;
		DWORD dwLen = NULL;

		bStop = !pThis->m_Grab.GetFrame(&Buffer, &dwLen);
		if (!bStop)
			pThis->Send(CAMERA_FRAME, Buffer, dwLen);	//编码后的,不用free

		//采集速度好像就是30fps/s,这里不用限制了,
		//网络延迟加队列缓存造成服务器不能实时显示画面,图像越大,由于带宽较小,延迟越高
		/*DWORD dwUsedTime = (GetTickCount() - dwLastTime);
		while ((GetTickCount() - dwLastTime) < (1000 / 30))
			Sleep(1);*/
	}
	InterlockedExchange(&pThis->m_bStop, TRUE);
}

void CCamera::OnStop()
{
	m_Grab.StopGrab();						//停止捕捉
	InterlockedExchange(&m_bStop, TRUE);	//停止发送

	if (m_hWorkThread)
	{
		WaitForSingleObject(m_hWorkThread,INFINITE);		//等待线程退出
		CloseHandle(m_hWorkThread);
		m_hWorkThread = NULL;
	}
	//关闭完成.
	m_Grab.GrabberTerm();

	printf("Send Camera Top OK!\n");
	Send(CAMERA_STOP_OK, 0, 0);
}

void CCamera::OnReadComplete(WORD Event, DWORD Total, DWORD dwRead, char*Buffer)
{
	switch (Event)
	{
	case CAMERA_START:
		OnStart(((DWORD*)Buffer)[0], ((DWORD*)Buffer)[1], ((DWORD*)Buffer)[2]);
		break;
	case CAMERA_STOP:
		OnStop();
		break;
	case CAMERA_SCREENSHOT:
		Send(CAMERA_SCREENSHOT, 0, 0);
		break;
	default:
		break;
	}
}