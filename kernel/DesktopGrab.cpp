#include "DesktopGrab.h"
#include <stdio.h>
#pragma comment(lib,"yuv.lib")

CDesktopGrab::CDesktopGrab()
{
	m_dwWidth = 0;
	m_dwHeight = 0;

	m_dwScreenWidth = 0;		//..
	m_dwScreenHeight = 0;

	m_hDC = NULL;
	m_hMemDC = NULL;
	
	m_hBmp = NULL;
	memset(&m_Bmp, 0, sizeof(m_Bmp));
	m_Buffer = NULL;
	m_dwBpp = 0;

	m_pEncoder = NULL;
	m_pPicIn = NULL;
	m_pPicOut = NULL;
}


CDesktopGrab::~CDesktopGrab()
{
	GrabTerm();
}

void CDesktopGrab::GrabTerm()
{
	//
	if (m_pEncoder)
	{
		_x264_encoder_close(m_pEncoder);
		m_pEncoder = NULL;
	}
	if (m_pPicIn)
	{
		_x264_picture_clean(m_pPicIn);
		free(m_pPicIn);
		m_pPicIn = NULL;
	}
	if (m_pPicOut)
	{
		free(m_pPicOut);
		m_pPicOut = NULL;
	}
	//
	if (m_hMemDC)
	{
		DeleteDC(m_hMemDC);
		m_hMemDC = NULL;
	}
	if (m_hBmp)
	{
		DeleteObject(m_hBmp);
		m_hBmp = NULL;
		m_Buffer = NULL;
		memset(&m_Bmp, 0, sizeof(BITMAP));
	}
	if (m_hDC)
	{
		ReleaseDC(NULL,m_hDC);
		m_hDC = NULL;
	}
	m_dwWidth = NULL;
	m_dwHeight = NULL;
	m_dwScreenHeight = 0;
	m_dwScreenWidth = 0;
}
BOOL CDesktopGrab::GrabInit()
{
	x264_param_t param = { 0 };
	BITMAPINFO bmi = { 0 };
	//先把之前的资源释放.
	GrabTerm();
	//
	m_hDC = GetDC(NULL);
	if (m_hDC == NULL)
	{	
		printf("GetDC Failed!\n");
		goto Error;
	}
	m_dwHeight = GetDeviceCaps(m_hDC, DESKTOPVERTRES);
	m_dwWidth = GetDeviceCaps(m_hDC, DESKTOPHORZRES);
	m_dwBpp = GetDeviceCaps(m_hDC, BITSPIXEL);

	//printf("Width:%d Height:%d\n",m_dwWidth,m_dwHeight);
	

	m_dwScreenHeight = GetDeviceCaps(m_hDC, VERTRES);
	m_dwScreenWidth = GetDeviceCaps(m_hDC, HORZRES);
	//
	m_hMemDC = CreateCompatibleDC(m_hDC);

	if (m_hMemDC == NULL)
	{	
		printf("Create Compatible DC Failed!\n");
		goto Error;
	
	}//
	
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = m_dwWidth;
	bmi.bmiHeader.biHeight = m_dwHeight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = m_dwBpp;
	bmi.bmiHeader.biCompression = BI_RGB;
	//
	m_hBmp = CreateDIBSection(m_hMemDC, &bmi, DIB_RGB_COLORS, &m_Buffer, 0, 0);
	if (m_hBmp == NULL || !SelectObject(m_hMemDC, m_hBmp))
	{	
		printf("Create DIB Section Failed! || SelectObject Failed!\n");
		goto Error;
	}//
	GetObject(m_hBmp, sizeof(BITMAP), &m_Bmp);
	//
	//
	m_pPicIn = (x264_picture_t*)malloc(sizeof(x264_picture_t));
	m_pPicOut = (x264_picture_t*)malloc(sizeof(x264_picture_t));
	//保存截取的图像.
	_x264_picture_init(m_pPicIn);
	_x264_picture_alloc(m_pPicIn, X264_CSP_I420,m_dwWidth&0xfffffffe,m_dwHeight&0xfffffffe);
	//fast 和veryfast速度差了好多........
	_x264_param_default_preset(&param, "veryfast", "zerolatency");

	param.i_width = m_dwWidth&0xfffffffe;
	param.i_height = m_dwHeight&0xfffffffe;
	
	//x264_LOG_NONE
	param.i_log_level = X264_LOG_NONE;
	param.i_threads = 1;
	param.i_frame_total = 0;
	param.i_keyint_max = 10;
	param.i_bframe = 0;					//不启用b帧
	param.b_open_gop = 0;
	param.i_fps_num = 1;
	param.i_csp = X264_CSP_I420;
	_x264_param_apply_profile(&param, x264_profile_names[0]);

	m_pEncoder = _x264_encoder_open_163(&param);
	if (m_pEncoder == NULL)
	{	
		printf("_x264_encoder_open_163 failed!\n");
		goto Error;
	}
	return TRUE;
Error:
	GrabTerm();
	return FALSE;
}
void CDesktopGrab::GetDesktopSize(DWORD *pWidth, DWORD*pHeight)
{
	*pWidth = m_dwWidth&0xfffffffe;
	*pHeight = m_dwHeight&0xfffffffe;
}
BOOL CDesktopGrab::GetFrame(char**ppbuffer, DWORD*pSize, BOOL bGrabMouse, BOOL bGrabTransparentWnd)
{
	*ppbuffer = NULL;
	*pSize = NULL;
	BOOL	bResult = FALSE;
	
	//失败。
	if (m_pEncoder == NULL)
		return FALSE;
	//当分辨率变化的时候截取会不完整.

	DWORD dwFlag = SRCCOPY;
	if (bGrabTransparentWnd)
		dwFlag |= CAPTUREBLT;
	//截图
	if (FALSE == BitBlt(m_hMemDC, 0, 0, m_dwWidth, m_dwHeight, m_hDC, 0, 0, dwFlag))
		return FALSE;
	//绘制鼠标
	if (bGrabMouse)
		DrawMouse();

	//转为yuv420,rgb是倒过来的.
	switch (m_dwBpp)
	{
	case 24:
		libyuv::RGB24ToI420((uint8_t*)m_Buffer, m_Bmp.bmWidthBytes, m_pPicIn->img.plane[0], m_pPicIn->img.i_stride[0],
			m_pPicIn->img.plane[1], m_pPicIn->img.i_stride[1], m_pPicIn->img.plane[2], m_pPicIn->img.i_stride[2], m_dwWidth&0xfffffffe, m_dwHeight&0xfffffffe);
		break;
	case 32:
		libyuv::ARGBToI420((uint8_t*)m_Buffer, m_Bmp.bmWidthBytes, m_pPicIn->img.plane[0], m_pPicIn->img.i_stride[0],
			m_pPicIn->img.plane[1], m_pPicIn->img.i_stride[1], m_pPicIn->img.plane[2], m_pPicIn->img.i_stride[2], m_dwWidth&0xfffffffe, m_dwHeight&0xfffffffe);
		break;
	default:
		//只支持24,32位位图.
		return FALSE;
	}
	//编码一帧数据
	x264_nal_t *pNal;
	int			iNal;
	*pSize = _x264_encoder_encode(m_pEncoder, &pNal, &iNal, m_pPicIn, m_pPicOut);
	*ppbuffer = (char*)pNal[0].p_payload;
	bResult = (*ppbuffer != NULL);

	return bResult;
}

//以下代码来自ffmpeg.

void CDesktopGrab::DrawMouse()
{
	CURSORINFO ci = { 0 };
	ci.cbSize = sizeof(ci);

	if (GetCursorInfo(&ci))
	{
		HCURSOR icon = CopyCursor(ci.hCursor);
		ICONINFO info;
		info.hbmMask = NULL;
		info.hbmColor = NULL;
		//貌似是现在没有光标
		if (ci.flags != CURSOR_SHOWING)
			return;

		if (!icon) {
			icon = CopyCursor(LoadCursor(NULL, IDC_ARROW));
		}
		if (!GetIconInfo(icon, &info)) {
			goto icon_error;
		}
		
		DrawIcon(m_hMemDC, m_dwWidth *ci.ptScreenPos.x / m_dwScreenWidth,m_dwHeight * ci.ptScreenPos.y/m_dwScreenHeight, icon);
icon_error:
		if (info.hbmMask)
			DeleteObject(info.hbmMask);
		if (info.hbmColor)
			DeleteObject(info.hbmColor);
		if (icon)
			DestroyCursor(icon);
	}
}