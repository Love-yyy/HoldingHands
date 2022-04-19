#include "CameraGrab.h"

#pragma comment(lib, "Strmiids.lib")
#pragma comment(lib, "yuv.lib")


CCameraGrab::CCameraGrab()
{
	m_dwHeight = 0;
	m_dwWidth = 0;

	m_hEvent = NULL;
	m_hMutex = NULL;

	m_pGraph = NULL;
	m_pBuild = NULL;
	m_pCaptureFilter = NULL;

	m_pSampleGrabberFilter = NULL;
	m_pSamplerGrabber = NULL;
	m_pNullRenderer = NULL;
	m_pMediaControl = NULL;

	m_pCallback = NULL;
	m_dwBufSize = NULL;

	m_pFrameListHead = m_pFrameListTail = NULL;
	//
	m_pEncoder = NULL;
	m_pPicOut = NULL;
	m_pPicIn = NULL;

}


CCameraGrab::~CCameraGrab()
{
	printf("CCameraGrab::~CCameraGrab()\n");
	GrabberTerm();
	printf("CCameraGrab::~CCameraGrab() OK \n");
}

void CCameraGrab::GrabberTerm()
{
	printf("Grabber Term start\n");
	if (m_pMediaControl)
	{
		m_pMediaControl->Stop();
		m_pMediaControl->Release();
		m_pMediaControl = NULL;
	}
	if (m_hMutex)
	{
		CloseHandle(m_hMutex);
		m_hMutex = NULL;
	}
	if (m_hEvent)
	{
		CloseHandle(m_hEvent);
		m_hEvent = NULL;
	}

	if (m_pGraph)
	{
		m_pGraph->Release();
		m_pGraph = NULL;
	}
	if (m_pBuild)
	{
		m_pBuild->Release();
		m_pBuild = NULL;
	}
	if (m_pCaptureFilter)
	{
		m_pCaptureFilter->Release();
		m_pCaptureFilter = NULL;
	}

	if (m_pSamplerGrabber)
	{
		m_pSamplerGrabber->Release();
		m_pSamplerGrabber = NULL;
	}

	if (m_pSampleGrabberFilter)
	{
		m_pSampleGrabberFilter->Release();
		m_pSampleGrabberFilter = NULL;
	}
	if (m_pNullRenderer)
	{
		m_pNullRenderer->Release();
		m_pNullRenderer = NULL;
	}
	if (m_pCallback)
	{
		delete m_pCallback;
		m_pCallback = NULL;
	}
	FrameList*p = m_pFrameListHead;
	while (p)
	{
		FrameList*pTemp = p;
		p = p->next;
		free(pTemp->frame.m_buffer);
		free(pTemp);
	}
	m_pFrameListHead = NULL;
	m_pFrameListTail = NULL;
	m_dwBufSize = NULL;

	//x264
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
	m_dwHeight = 0;
	m_dwWidth = 0;
	printf("Grabber Term Ok\n");
}

void CCameraGrab::AddTail(char*buffer,DWORD dwLen)
{
	if (m_dwBufSize > MAX_BUF_SIZE)
		return;
	
	FrameList*pNewFrame = (FrameList*)malloc(sizeof(FrameList));
	pNewFrame->next = NULL;
	pNewFrame->frame.m_buffer = (char*)malloc(dwLen);
	pNewFrame->frame.m_dwLength = dwLen;

	memcpy(pNewFrame->frame.m_buffer, buffer, dwLen);

	WaitForSingleObject(m_hMutex, INFINITE);

	m_dwBufSize += dwLen;
	if (m_pFrameListHead == NULL)
	{
		m_pFrameListHead = m_pFrameListTail = pNewFrame;
	}
	else
	{
		m_pFrameListTail->next = pNewFrame;
		m_pFrameListTail = pNewFrame;
	}
	SetEvent(m_hEvent);

	SetEvent(m_hMutex);
}
void CCameraGrab::RemoveHead(char**ppBuffer,DWORD*pLen)
{
	printf("Remove Head\n");
	FrameList*pFrame = NULL;
	while (true)
	{
		WaitForSingleObject(m_hMutex, INFINITE);
		pFrame = m_pFrameListHead;

		if (m_pFrameListHead == m_pFrameListTail)
			m_pFrameListHead = m_pFrameListTail = NULL;
		else
			m_pFrameListHead = m_pFrameListHead->next;

		if (!pFrame)
			ResetEvent(m_hEvent);
		else
			m_dwBufSize -= pFrame->frame.m_dwLength;

		SetEvent(m_hMutex);

		if (!pFrame)
		{
			printf("No Frame,Wait ForSingle Object\n");
			if (WAIT_TIMEOUT == WaitForSingleObject(m_hEvent, 3000))
			{
				printf("Wait For Single Object Time Out\n");
				*ppBuffer = NULL;
				*pLen = NULL;
				return;
			}
		}
		else
			break;
	}
	*ppBuffer = pFrame->frame.m_buffer;
	*pLen = pFrame->frame.m_dwLength;
	free(pFrame);
}

list<CCameraGrab::DeviceInfo>& CCameraGrab::GetDeviceList()
{
	m_DeviceList.clear();
	GetDeviceList(-1, 0);
	return m_DeviceList;
}

void CCameraGrab::GetDeviceList(int DeviceIdx, IBaseFilter**ppBaseFilter)
{
	HRESULT hr = NULL;
	ICreateDevEnum*pDevEnum = NULL;
	IEnumMoniker*pEnumMoniker = NULL;
	IMoniker*pMoniker = NULL;
	BOOL	bLoop = TRUE;
	int i = 0;

	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void **)&pDevEnum);
	if (FAILED(hr) || !pDevEnum)
	{	
		goto Failed;
	}
	hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumMoniker, 0);
	//
	if (FAILED(hr) || !pEnumMoniker)
	{	
		goto Failed;
	}
	while (hr = pEnumMoniker->Next(1, &pMoniker, NULL), hr == S_OK&& pMoniker&&bLoop)
	{
		IPropertyBag *pProBag;

		hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (LPVOID*)&pProBag);
		if (SUCCEEDED(hr) && pProBag)
		{
			VARIANT varTemp;
			varTemp.vt = VT_BSTR;
			hr = pProBag->Read(L"FriendlyName", &varTemp, NULL);
			if (SUCCEEDED(hr) && DeviceIdx == -1)
			{
				CCameraGrab::DeviceInfo device;
				wcscpy(device.szName, varTemp.bstrVal);

				IBaseFilter*	pBaseFilter = NULL;
				hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (LPVOID*)&pBaseFilter);
				if (SUCCEEDED(hr) && pBaseFilter)
				{
					IEnumPins* pIEnumPins = 0;
					IPin *pIPin = NULL;
					hr = pBaseFilter->EnumPins(&pIEnumPins);

					while (pIEnumPins && (hr = pIEnumPins->Next(1, &pIPin, NULL), SUCCEEDED(hr)) && pIPin)
					{
						IEnumMediaTypes* pMt = NULL;
						if (pIPin && SUCCEEDED(hr) && (hr = pIPin->EnumMediaTypes(&pMt), SUCCEEDED(hr)))
						{
							AM_MEDIA_TYPE *pMediaType = NULL;
							while (pMt && (hr = pMt->Next(1, &pMediaType, NULL), SUCCEEDED(hr)) && pMediaType)
							{
								if (pMediaType->formattype == FORMAT_VideoInfo && pMediaType->cbFormat >= (sizeof(VIDEOINFOHEADER))
									&& pMediaType->pbFormat)
								{
									VIDEOINFOHEADER* pVideoInfoHeader = (VIDEOINFOHEADER*)pMediaType->pbFormat;
									

									if (device.VideoSize.end() == device.VideoSize.find(pVideoInfoHeader->bmiHeader.biWidth))
										device.VideoSize.insert(pair<DWORD, DWORD>(pVideoInfoHeader->bmiHeader.biWidth, pVideoInfoHeader->bmiHeader.biHeight));
								}

								if (pMediaType->cbFormat && pMediaType->pbFormat)
								{
									CoTaskMemFree(pMediaType->pbFormat);
									pMediaType->pbFormat = NULL;
									pMediaType->cbFormat = NULL;
								}
								if (pMediaType->pUnk)
								{
									pMediaType->pUnk->Release();
									pMediaType->pUnk = NULL;
								}
								pMediaType = NULL;
							}
						}
						if (pIPin)
						{
							pIPin->Release();
							pIPin = NULL;
						}
					}
					if (pIEnumPins)
					{
						pIEnumPins->Release();
						pIEnumPins = NULL;
					}
					pBaseFilter->Release();
				}

				SysFreeString(varTemp.bstrVal);
				m_DeviceList.push_back(device);

			}
			pProBag->Release();
			//为选择的IDX
			if (i == DeviceIdx && ppBaseFilter)
			{
				bLoop = FALSE;
				IBaseFilter*	pBaseFilter = NULL;
				hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (LPVOID*)&pBaseFilter);
				if (SUCCEEDED(hr))
				{
					*ppBaseFilter = pBaseFilter;
				}
			}
			i++;
		}
		pMoniker->Release();
	}
Failed:
	if (pEnumMoniker)
		pEnumMoniker->Release();
	if (pDevEnum)
		pDevEnum->Release();
}

BOOL CCameraGrab::GrabberInit(int idx,DWORD dwWidth,DWORD dwHeight)
{
	HRESULT hr = NULL;
	AM_MEDIA_TYPE mt;
	AM_MEDIA_TYPE * mmt = NULL;
	IAMStreamConfig   *pConfig = NULL;
	VIDEOINFOHEADER * pvih = NULL;
	DWORD dwCompression;
	DWORD	dwBitCount;
	x264_param_t param = { 0 };

	GrabberTerm();

	m_dwHeight = dwHeight;
	m_dwWidth = dwWidth;
	//
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&m_pGraph);
	if (FAILED(hr))
		goto Failed;
	//
	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&m_pBuild);
	if (FAILED(hr))
		goto Failed;
	//
	hr = m_pBuild->SetFiltergraph(m_pGraph);
	if (FAILED(hr))
		goto Failed;
	//
	GetDeviceList(idx, &m_pCaptureFilter);
	if (!m_pCaptureFilter || (hr = m_pGraph->AddFilter(m_pCaptureFilter, L"Video Capture"), FAILED(hr)))
		goto Failed;
	//
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, (LPVOID*)&m_pSampleGrabberFilter);
	if (FAILED(hr))
		goto Failed;
	//
	hr = m_pSampleGrabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&m_pSamplerGrabber);
	if (FAILED(hr))
		goto Failed;
	//
	
	ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
	//设置输出视频格式.
	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_ARGB32;
	//
	m_pSamplerGrabber->SetMediaType(&mt);
	m_pSamplerGrabber->SetOneShot(FALSE);
	m_pSamplerGrabber->SetBufferSamples(FALSE);
	m_pCallback = new CMySampleGrabberCB(this);
	m_pSamplerGrabber->SetCallback(m_pCallback, 1);
	//
	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, (LPVOID*)&m_pNullRenderer);
	if (FAILED(hr))
		goto Failed;
	//
	hr = m_pGraph->AddFilter(m_pSampleGrabberFilter, L"Sample Filter");
	if (FAILED(hr))
		goto Failed;
	hr = m_pGraph->AddFilter(m_pNullRenderer, L"Null Renderer");
	if (FAILED(hr))
		goto Failed;
	//
	hr = m_pBuild->FindInterface(&PIN_CATEGORY_CAPTURE,
		&MEDIATYPE_Video, m_pCaptureFilter, IID_IAMStreamConfig, (void **)&pConfig);
	if (FAILED(hr))
		goto Failed;
	//获取视频尺寸,先设置完尺寸再连接起来。
	hr = pConfig->GetFormat(&mmt);    //取得默认参数
	if (FAILED(hr))
		goto Failed;
	//Set Capture Video Size;
	pvih = (VIDEOINFOHEADER*)mmt->pbFormat;
	pvih->bmiHeader.biHeight = dwHeight;
	pvih->bmiHeader.biWidth = dwWidth;
	dwBitCount = pvih->bmiHeader.biBitCount;
	dwCompression = pvih->bmiHeader.biCompression;
	//并不总是能成功.
	pvih->bmiHeader.biBitCount = 32;
	pvih->bmiHeader.biCompression = BI_RGB;
	//
	hr = pConfig->SetFormat(mmt);
	if (FAILED(hr))
	{
		printf("Set Format Failed!\n");
		pvih->bmiHeader.biBitCount = dwBitCount;
		pvih->bmiHeader.biCompression = dwCompression;
		hr = pConfig->SetFormat(mmt);
	}
	if (mmt->cbFormat && mmt->pbFormat)
	{
		CoTaskMemFree(mmt->pbFormat);
		mmt->pbFormat = NULL;
		mmt->cbFormat = NULL;
	}
	if (mmt->pUnk)
	{
		mmt->pUnk->Release();
		mmt->pUnk = NULL;
	}
	pConfig->Release();
	if (FAILED(hr))
	{
		printf("Set Format Failed!\n");
		goto Failed;
	}
	//
	hr = m_pBuild->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, m_pCaptureFilter, 
		m_pSampleGrabberFilter, m_pNullRenderer);
	if (FAILED(hr))
		goto Failed;
	//获取控制接口
	hr = m_pGraph->QueryInterface(IID_IMediaControl, (void**)&m_pMediaControl);
	if (FAILED(hr))
		goto Failed;
	//
	
	//初始化h264编码器.
	m_pPicIn = (x264_picture_t*)malloc(sizeof(x264_picture_t));
	m_pPicOut = (x264_picture_t*)malloc(sizeof(x264_picture_t));

	_x264_picture_init(m_pPicIn);
	_x264_picture_alloc(m_pPicIn, X264_CSP_I420, m_dwWidth, m_dwHeight);

	_x264_param_default_preset(&param, "veryfast", "zerolatency");

	param.i_width = m_dwWidth;
	param.i_height = m_dwHeight;

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
	if (!m_pEncoder)
	{	
		printf("x264 init Failed!\n");
		goto Failed;
	}
	//
	m_hMutex = CreateEvent(NULL, FALSE, TRUE, NULL);
	m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	return TRUE;
Failed:
	GrabberTerm();
	return FALSE;
}

BOOL CCameraGrab::GetFrame(char**ppbuffer,DWORD* plen)
{
	printf("Get Frame()\n");
	char*Buffer = NULL;
	DWORD dwLen = NULL;
	RemoveHead(&Buffer, &dwLen);
	if (Buffer == NULL)
	{
		printf("No Buffer\n");
		return FALSE;
	}

	libyuv::ARGBToI420((uint8_t*)Buffer, m_dwWidth * 4, m_pPicIn->img.plane[0], m_pPicIn->img.i_stride[0],
		m_pPicIn->img.plane[1], m_pPicIn->img.i_stride[1], m_pPicIn->img.plane[2], m_pPicIn->img.i_stride[2],m_dwWidth,m_dwHeight);
	x264_nal_t*pNal = NULL;
	int			iNal = 0;
	int			Size = 0;
	Size = _x264_encoder_encode(m_pEncoder, &pNal, &iNal, m_pPicIn, m_pPicOut);
	free(Buffer);

	if (Size < 0)
		return FALSE;
	*ppbuffer = (char*)pNal[0].p_payload;
	*plen = Size;
	return TRUE;
}
BOOL CCameraGrab::StartGrab()
{
	HRESULT hr = 0;
	if (m_pMediaControl && (hr = m_pMediaControl->Run(), SUCCEEDED(hr)))
		return TRUE;

	return FALSE;
}

void CCameraGrab::StopGrab()
{
	printf("On Stop\n");
	if (m_pMediaControl)
	{
		m_pMediaControl->Stop();
		printf("Stop Finished!\n");
	}
}