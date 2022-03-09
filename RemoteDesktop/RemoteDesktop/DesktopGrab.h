#pragma once
#include <stdint.h>

#include <Windows.h>
extern "C"{
#include <x264_config.h>
#include <x264.h>
#include <libyuv.h>
}

class CDesktopGrab
{
private:
	DWORD	m_dwWidth;				//�ֱ���
	DWORD	m_dwHeight;

	DWORD	m_dwScreenWidth;		//..
	DWORD	m_dwScreenHeight;

	HDC		m_hDC;
	HDC		m_hMemDC;
	HBITMAP	m_hBmp;
	BITMAP	m_Bmp;
	void*	m_Buffer;

	DWORD	m_dwBpp;		//bits per pix.
	x264_t*	m_pEncoder;		//������ʵ��

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

