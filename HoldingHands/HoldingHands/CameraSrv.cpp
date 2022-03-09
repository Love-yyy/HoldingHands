#include "stdafx.h"
#include "CameraSrv.h"
#include "CameraDlg.h"

CCameraSrv::CCameraSrv(DWORD dwIdentity) :
CEventHandler(dwIdentity)
{
	m_pDlg = NULL;
	
	m_pCodec = NULL;
	m_pCodecContext = NULL;
	
	m_hBmp = NULL;
	m_hMemDC = NULL;
	m_Buffer = NULL;

	memset(&m_Bmp, 0, sizeof(m_Bmp));
	memset(&m_AVPacket, 0, sizeof(m_AVPacket));
	memset(&m_AVFrame, 0, sizeof(m_AVFrame));

}


CCameraSrv::~CCameraSrv()
{
}

void CCameraSrv::OnClose()
{
	if (m_pDlg)
	{
		delete m_pDlg;
	}
	CameraTerm();
}
void CCameraSrv::OnConnect()
{
	m_pDlg = new CCameraDlg(this);
	if (FALSE == m_pDlg->Create(IDD_CAM_DLG))
	{
		Disconnect();
		return;
	}
	m_pDlg->ShowWindow(SW_SHOW);
}
void CCameraSrv::OnReadComplete(WORD Event, DWORD Total, DWORD dwRead, char*Buffer)
{
	switch (Event)
	{
	case CAMERA_DEVICELIST:
		OnDeviceList(Buffer);
		break;
	case CAMERA_VIDEOSIZE:
		OnVideoSize((DWORD*)Buffer);
		break;
	case CAMERA_FRAME:
		OnFrame(Buffer,dwRead);
		break;
	case CAMERA_SCREENSHOT:
		OnScreenShot();
		break;
	case CAMERA_ERROR:
		OnError((WCHAR*)Buffer);
		break;
	default:
		break;
	}
}

void CCameraSrv::OnDeviceList(char*DeviceList)
{
	m_pDlg->SendMessage(WM_CAMERA_DEVICELIST, (WPARAM)DeviceList, NULL);
}

void CCameraSrv::Start(int idx, DWORD dwWidth, DWORD dwHeight)
{
	CameraTerm();
	DWORD dwParams[3] = { idx, dwWidth, dwHeight };
	Send(CAMERA_START, (char*)&dwParams, sizeof(dwParams));
}
void CCameraSrv::Stop()
{
	Send(CAMERA_STOP, 0, 0);
	CameraTerm();
}

BOOL CCameraSrv::CameraInit(DWORD dwHeight,DWORD dwWidth)
{
	CameraTerm();
	BITMAPINFO bmi = { 0 };
	CDC*pDc = m_pDlg->GetDC();
	//创建内存dc
	m_hMemDC = CreateCompatibleDC(pDc->GetSafeHdc());
	if (m_hMemDC == NULL)
		goto Failed;
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = dwWidth;
	bmi.bmiHeader.biHeight = dwHeight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;				//4字节貌似解码快
	bmi.bmiHeader.biCompression = BI_RGB;

	//创建DIBSection
	m_hBmp = CreateDIBSection(m_hMemDC, &bmi, DIB_RGB_COLORS, &m_Buffer, 0, 0);
	if (m_hBmp == NULL || !SelectObject(m_hMemDC, m_hBmp))
		goto Failed;
	//
	GetObject(m_hBmp, sizeof(BITMAP), &m_Bmp);
	//
	//创建解码器.
	m_pCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (m_pCodec == NULL)
		goto Failed;
	//
	m_pCodecContext = avcodec_alloc_context3(m_pCodec);
	if (m_pCodecContext == NULL)
		goto Failed;
	//
	if (0 != avcodec_open2(m_pCodecContext, m_pCodec, 0))
		goto Failed;
	return TRUE;
Failed:
	CameraTerm();
	return FALSE;
}
void CCameraSrv::CameraTerm()
{
	if (m_hMemDC)
	{
		DeleteDC(m_hMemDC);
		m_hMemDC = NULL;
	}
	if (m_hBmp)
	{
		DeleteObject(m_hBmp);
		memset(&m_Bmp, 0, sizeof(m_Bmp));
		m_hBmp = NULL;
	}
	if (m_pCodecContext)
	{
		avcodec_free_context(&m_pCodecContext);
		m_pCodecContext = 0;
	}
	m_pCodec = 0;
	//AVFrame需要清除
	av_frame_unref(&m_AVFrame);
	//
	memset(&m_AVPacket, 0, sizeof(m_AVPacket));
	memset(&m_AVFrame, 0, sizeof(m_AVFrame));
}

void CCameraSrv::OnVideoSize(DWORD dwVideoSize[])
{
	if (!dwVideoSize[0])
	{
		m_pDlg->SendMessage(WM_CAMERA_ERROR, (WPARAM)L"Grabber Init Failed!", NULL);
		return;
	}
	if (FALSE == CameraInit(dwVideoSize[2], dwVideoSize[1]))
	{
		m_pDlg->SendMessage(WM_CAMERA_ERROR, (WPARAM)L"CameraSrv Init Failed!", NULL);
		return;
	}
	m_pDlg->SendMessage(WM_CAMERA_VIDEOSIZE, dwVideoSize[1], dwVideoSize[2]);
	//Send(CAMERA_GETFRAME, 0, 0);
}

void CCameraSrv::OnFrame(char*buffer,DWORD dwLen)
{
	if (m_pCodecContext == NULL)
	{
		//解码器还没有创建,丢弃
		return;
	}

	av_init_packet(&m_AVPacket);
	m_AVPacket.data = (uint8_t*)buffer;
	m_AVPacket.size = dwLen;

	//Send(CAMERA_GETFRAME, 0, 0);
	if (!avcodec_send_packet(m_pCodecContext, &m_AVPacket))
	{
		if (!avcodec_receive_frame(m_pCodecContext, &m_AVFrame))
		{
			//成功.
			//I420 ---> ARGB.
			libyuv::I420ToARGB(m_AVFrame.data[0], m_AVFrame.linesize[0], m_AVFrame.data[1], m_AVFrame.linesize[1],
				m_AVFrame.data[2], m_AVFrame.linesize[2],
				(uint8_t*)m_Buffer, m_Bmp.bmWidthBytes, m_Bmp.bmWidth, m_Bmp.bmHeight);
			//显示到窗口上
			m_pDlg->SendMessage(WM_CAMERA_FRAME, (WPARAM)m_hMemDC, (LPARAM)&m_Bmp);
			return;
		}
	}
	m_pDlg->SendMessage(WM_CAMERA_ERROR, (WPARAM)L"Decode Frame Failed!", NULL);
	return;
}

void CCameraSrv::ScreenShot()
{
	Send(CAMERA_SCREENSHOT, 0, 0);
}

void CCameraSrv::OnScreenShot()
{
	m_pDlg->SendMessage(WM_CAMERA_SCREENSHOT, 0, 0);
}

void CCameraSrv::OnError(WCHAR*szError)
{
	m_pDlg->SendMessage(WM_CAMERA_ERROR, (WPARAM)szError, 0);
}