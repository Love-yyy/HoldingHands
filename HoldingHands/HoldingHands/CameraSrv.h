#pragma once
#include "EventHandler.h"
#include "resource.h"

extern"C"
{
#include <libavcodec\avcodec.h>
#include <libavutil\avutil.h>
#include <libyuv.h>
}


#define CAMERA			('C'|('A'<<8)|('M'<<16)|('E')<<24)
#define CAMERA_DEVICELIST			(0xabc1)
#define CAMERA_START				(0xabc2)
#define CAMERA_VIDEOSIZE			(0xabc3)
#define CAMERA_STOP					(0xabc4)

#define CAMERA_GETFRAME				(0xabc5)
#define CAMERA_FRAME				(0xabc6)
#define CAMERA_SCREENSHOT			(0xabc7)

#define CAMERA_ERROR				(0x0000)

class CCameraDlg;

class CCameraSrv :
	public CEventHandler
{
private:
	CCameraDlg*	m_pDlg;
	
	AVCodec*			m_pCodec;
	AVCodecContext*		m_pCodecContext;
	AVPacket			m_AVPacket;
	AVFrame				m_AVFrame;

	HBITMAP				m_hBmp;
	BITMAP				m_Bmp;
	HDC					m_hMemDC;
	void*				m_Buffer;

	BOOL	CameraInit(DWORD dwHeight, DWORD dwWidth);
	void	CameraTerm();
public:
	void OnConnect();
	void OnClose();
	void OnReadComplete(WORD Event, DWORD Total, DWORD dwRead, char*Buffer);

	void OnFrame(char*Buffer, DWORD dwLen);
	void OnVideoSize(DWORD dwVideoSize[]);
	void OnDeviceList(char*DeviceList);
	void OnScreenShot();


	void Start(int idx,DWORD dwWidth,DWORD dwHeight);
	void Stop();
	void ScreenShot();
	void OnError(WCHAR*szError);

	CCameraSrv(DWORD dwIdentity);
	~CCameraSrv();
};

