#pragma once

#define STRSAFE_NO_DEPRECATE
#include <Windows.h>
#include <dshow.h>
#include "qedit.h"
#include "MySampleGrabberCB.h"
#include <map>
#include <string>
#include <list>
using namespace std;

#include "x264EncoderLoad.h"

extern "C"{
#include "libyuv.h"
}





//最大缓冲40MB,足够一张图片了.
#define MAX_BUF_SIZE (40*1024*1024)
class CCameraGrab
{
public:
	struct Frame
	{
		char*	m_buffer;
		DWORD	m_dwLength;
	};
	
	typedef struct FrameList
	{
		Frame		frame;
		FrameList*	next;
	}FrameList;
	
	typedef struct DeviceInfo
	{
		WCHAR szName[128];
		map<DWORD, DWORD> VideoSize;
	}DeviceInfo;

private:	

	friend class CMySampleGrabberCB;

	DWORD m_dwHeight;
	DWORD m_dwWidth;

	HANDLE	m_hEvent;						//标记FrameList 是否为空
	HANDLE	m_hMutex;

	FrameList*		m_pFrameListHead;
	FrameList*		m_pFrameListTail;

	DWORD			m_dwBufSize;
	void AddTail(char*, DWORD);
	void RemoveHead(char**ppBuffer, DWORD*pLen);

	IGraphBuilder		  *m_pGraph;
	ICaptureGraphBuilder2 *m_pBuild;
	IBaseFilter			  *m_pCaptureFilter;
	IBaseFilter			  *m_pSampleGrabberFilter;
	ISampleGrabber		  *m_pSamplerGrabber;
	IBaseFilter		      *m_pNullRenderer;
	IMediaControl		  *m_pMediaControl;
	CMySampleGrabberCB	  *m_pCallback;
	//x264
	x264_t*	m_pEncoder;		//编码器实例

	x264_picture_t *m_pPicIn;
	x264_picture_t *m_pPicOut;


	list<DeviceInfo> m_DeviceList;
	
public:
	void GetDeviceList(int DeviceIdx, IBaseFilter**ppBaseFilter);
	list<DeviceInfo>& GetDeviceList();

	BOOL GrabberInit(int idx,DWORD dwWidth, DWORD dwHeight);
	BOOL StartGrab();
	void StopGrab();
	BOOL GetFrame(char**ppbuffer, DWORD* plen);

	void GrabberTerm();
	CCameraGrab();
	~CCameraGrab();
};

