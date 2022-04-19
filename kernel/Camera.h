#pragma once
#include "EventHandler.h"
#include "CameraGrab.h"
#define CAMERA			('C'|('A'<<8)|('M'<<16)|('E')<<24)
#define CAMERA_DEVICELIST			(0xabc1)
#define CAMERA_START				(0xabc2)
#define CAMERA_VIDEOSIZE			(0xabc3)
#define CAMERA_STOP					(0xabc4)

#define CAMERA_STOP_OK				(0xabdd)


#define CAMERA_FRAME				(0xabc6)
#define CAMERA_SCREENSHOT			(0xabc7)

#define CAMERA_ERROR				(0x0000)

class CCamera :
	public CEventHandler
{
private:
	CCameraGrab m_Grab;
	volatile long	m_bStop;

	HANDLE		m_hWorkThread;

public:

	void OnStart(int idx,DWORD dwWidth,DWORD dwHeight);
	void OnStop();

	void OnConnect();
	void OnClose();
	void OnReadComplete(WORD Event, DWORD Total, DWORD dwRead, char*Buffer);


	void static WorkThread(CCamera*pThis);

	CCamera(DWORD dwIdentity);
	~CCamera();
};

