#pragma once


#include <Windows.h>
#include "x264EncoderLoad.h"

extern "C"{
	#include "libyuv.h"
}

class CDesktopGrab
{
private:
	DWORD	m_dwWidth;				//·Ö±æÂÊ
	DWORD	m_dwHeight;

	DWORD	m_dwScreenWidth;		//..
	DWORD	m_dwScreenHeight;

	HDC		m_hDC;
	HDC		m_hMemDC;
	HBITMAP	m_hBmp;
	BITMAP	m_Bmp;
	void*	m_Buffer;

	DWORD	m_dwBpp;		//bits per pix.
	x264_t*	m_pEncoder;		//±àÂëÆ÷ÊµÀý

	x264_picture_t *m_pPicIn;
	x264_picture_t *m_pPicOut;

	
	void	DrawMouse();
public:
	BOOL	GrabInit();
	void	GrabTerm();
	void	GetDesktopSize(DWORD *pWidth, DWORD*pHeight);
	BOOL	GetFrame(char**ppbuffer,DWORD*pSize,BOOL bGrabMouse = FALSE,BOOL bGrabTransparentWnd = FALSE);
	CDesktopGrab();
	~CDesktopGrab();
};

