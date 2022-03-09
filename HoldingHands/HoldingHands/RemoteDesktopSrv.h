#pragma once
extern "C"
{
#include <libavcodec\avcodec.h>
#include <libavutil\avutil.h>
#include <libyuv.h>
}

#define REMOTEDESKTOP	('R'|('D'<<8)|('T'<<16)|('P'<<24))

#define REMOTEDESKTOP_GETSIZE		(0xaa01)
#define REMOTEDESKTOP_DESKSIZE		(0xaa02)

#define REMOTEDESKTOP_NEXT_FRAME			(0xaaa1)
#define REMOTEDESKTOP_FRAME			(0xaaa2)
#define REMOTEDESKTOP_ERROR			(0xaaa3)

//
#define REMOTEDESKTOP_CTRL			(0xaaa4)
#define REMOTEDESKTOP_SETMAXFPS		(0xaaa5)
#define REMOTEDESKTOP_SETFLAG		(0xaaa6)

#include "EventHandler.h"

class CRemoteDesktopWnd;

struct CtrlParam2
{
	DWORD m_dwCount;
	INPUT m_inputs[1];
};

class CRemoteDesktopSrv :
	public CEventHandler
{
private:
	AVCodec*			m_pCodec;
	AVCodecContext*		m_pCodecContext;
	AVPacket			m_AVPacket;
	AVFrame				m_AVFrame;

	HBITMAP				m_hBmp;
	BITMAP				m_Bmp;
	HDC					m_hMemDC;
	void*				m_Buffer;

	CRemoteDesktopWnd*	m_pWnd;

	BOOL RemoteDesktopSrvInit(DWORD dwWidth,DWORD dwHeight);
	void RemoteDesktopSrvTerm();
	
	//Event
	void OnDeskSize(char*DeskSize);
	void OnError(WCHAR* szError);
	void OnFrame(DWORD dwRead,char*Buffer);

public:
	
	//
	void NextFrame();
	void Control2(CtrlParam2*pParam);

	void SetMaxFps(DWORD dwMaxFps);
	void OnClose();					//当socket断开的时候调用这个函数
	void OnConnect();				//当socket连接的时候调用这个函数
	//有数据到达的时候调用这两个函数.
	void OnReadPartial(WORD Event, DWORD Total, DWORD nRead, char*Buffer);
	void OnReadComplete(WORD Event, DWORD Total, DWORD nRead, char*Buffer);
	//有数据发送完毕后调用这两个函数
	void OnWritePartial(WORD Event, DWORD Total, DWORD nWrite, char*Buffer);
	void OnWriteComplete(WORD Event, DWORD Total, DWORD nWrite, char*Buffer);
	CRemoteDesktopSrv(DWORD dwIdentidy);
	~CRemoteDesktopSrv();
};

