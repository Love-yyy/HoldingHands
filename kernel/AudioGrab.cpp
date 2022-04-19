#include "AudioGrab.h"
#include <stdio.h>
#pragma comment(lib,"Winmm.lib")
CAudioGrab::CAudioGrab()
{
	m_hEvent = NULL;
	m_hMutex = NULL;
	m_pListHead = NULL;
	m_pListTail = NULL;
	m_hWaveIn = NULL;
	m_dwBufSize = 0;
	m_hWorkThread = NULL;
	m_dwThreadID = NULL;
	m_Idx = 0;

	memset(&m_Wavefmt, 0, sizeof(m_Wavefmt));
	//带宽足够了,就不压缩了
	m_Wavefmt.wFormatTag = WAVE_FORMAT_PCM; // ACM will auto convert wave format
	m_Wavefmt.nChannels = 1;
	m_Wavefmt.nSamplesPerSec = 44100;
	m_Wavefmt.nAvgBytesPerSec = 44100 * 2;
	m_Wavefmt.nBlockAlign = 2;
	m_Wavefmt.wBitsPerSample = 16;
	m_Wavefmt.cbSize = 0;
}


CAudioGrab::~CAudioGrab()
{
	StopGrab();
}

void __stdcall CAudioGrab::WorkThread(CAudioGrab*pThis)
{
	//callback function会造成死锁.改为线程.
	MSG msg;
	WAVEHDR*pHdr = 0;
	MMRESULT mmResult = 0;
	while (GetMessage(&msg, 0, 0, 0))
	{
		switch (msg.message)
		{
		case MM_WIM_DATA:
			pHdr = &pThis->m_hdrs[pThis->m_Idx];
			pThis->AddTail(pHdr->lpData, pHdr->dwBytesRecorded);
			
			pHdr->dwBytesRecorded = 0;
			pHdr->dwFlags = 0;
			pHdr->dwLoops = 0;
			pHdr->dwUser = 0;
			pHdr->lpNext = 0;
			pHdr->reserved = 0;

			waveInPrepareHeader(pThis->m_hWaveIn, pHdr, sizeof(WAVEHDR));
			if (!mmResult)
				printf("waveInPrepareHeader Ok!\n");

			mmResult = waveInAddBuffer(pThis->m_hWaveIn, pHdr, sizeof(WAVEHDR));
			if (!mmResult)
				printf("waveInAddBuffer Ok!\n");
			pThis->m_Idx = (pThis->m_Idx + 1) % BUFF_COUNT;
			break;
		default:
			break;
		}
	}
	printf("Thread Exit\n");
}

BOOL CAudioGrab::GrabInit()
{
	MMRESULT mmResult = 0;
	int i;
	GrabTerm();
	//
	m_hWorkThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)WorkThread, this, 0, &m_dwThreadID);
	if (m_hWorkThread == NULL)
		goto Failed;

	mmResult = waveInOpen(&m_hWaveIn, WAVE_MAPPER, &m_Wavefmt, (DWORD_PTR)m_dwThreadID, (DWORD_PTR)this, CALLBACK_THREAD);
	//
	if (mmResult != MMSYSERR_NOERROR)
		goto Failed;

	memset(m_hdrs, 0, sizeof(m_hdrs));

	for (i = 0; i < BUFF_COUNT; i++)
	{
		m_hdrs[i].dwBufferLength = LEN_PER_BUFF;
		m_hdrs[i].lpData = m_Buffers[i];
		memset(m_Buffers, 0, LEN_PER_BUFF);

		mmResult = waveInPrepareHeader(m_hWaveIn, &m_hdrs[i], sizeof(WAVEHDR));
		if (mmResult != MMSYSERR_NOERROR)
			goto Failed;
		mmResult = waveInAddBuffer(m_hWaveIn, &m_hdrs[i], sizeof(WAVEHDR));
		if (mmResult != MMSYSERR_NOERROR)
			goto Failed;
	}
	m_hMutex = CreateEvent(NULL, FALSE, TRUE, NULL);
	m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	return TRUE;
Failed:
	GrabTerm();
	return FALSE;
}

void CAudioGrab::GrabTerm()
{
	//等待线程退出
	if (m_hWorkThread && m_dwThreadID)
	{
		PostThreadMessage(m_dwThreadID, WM_QUIT, 0, 0);
		WaitForSingleObject(m_hWorkThread, INFINITE);
		CloseHandle(m_hWorkThread);
	}
	m_hWorkThread = 0;
	m_dwThreadID = 0;
	//关闭设备
	if (m_hWaveIn)
	{
		waveInClose(m_hWaveIn);
		m_hWaveIn = NULL;
	}
	//
	if (m_hEvent)
	{
		CloseHandle(m_hEvent);
		m_hEvent = NULL;
	}
	if (m_hMutex)
	{
		CloseHandle(m_hMutex);
		m_hMutex = NULL;
	}
	//清理缓存
	while (m_pListHead)
	{
		AudioBuffList*pTemp = m_pListHead;
		m_pListHead = m_pListHead->next;
		free(pTemp->m_Buffer);
		free(pTemp);
	}
	m_pListHead = NULL;
	m_pListTail = NULL;
	//
	m_dwBufSize = 0;
	m_Idx = 0;
}


BOOL CAudioGrab::StartGrab()
{
	if (m_hWaveIn && !waveInStart(m_hWaveIn))
		return TRUE;
	return FALSE;
}
void CAudioGrab::StopGrab()
{
	if (m_hWaveIn)
		waveInStop(m_hWaveIn);
}
BOOL CAudioGrab::GetBuffer(void**ppBuffer, DWORD*pBufLen)
{
	printf("Get Buffer()\n");
	char*Buffer = NULL;
	DWORD dwLen = NULL;
	RemoveHead(&Buffer, &dwLen);
	if (Buffer == NULL)
	{
		printf("No Buffer\n");
		return FALSE;
	}
	*ppBuffer = Buffer;
	*pBufLen = dwLen;
	return TRUE;
}

void  CAudioGrab::RemoveHead(char**ppBuffer, DWORD*pLen)
{
	printf("Remove Head\n");
	AudioBuffList*pBuf = NULL;
	*ppBuffer = NULL;
	*pLen = NULL;
	while (true)
	{
		WaitForSingleObject(m_hMutex, INFINITE);
		pBuf = m_pListHead;

		if (m_pListHead == m_pListTail)
			m_pListHead = m_pListTail = NULL;
		else
			m_pListHead = m_pListHead->next;

		if (!pBuf)
			ResetEvent(m_hEvent);
		else
			m_dwBufSize -= pBuf->m_dwBuffLen;

		SetEvent(m_hMutex);

		if (!pBuf)
		{
			printf("No Buf,Wait ForSingle Object\n");
			if (WAIT_TIMEOUT == WaitForSingleObject(m_hEvent, 3000))
			{
				printf("Wait For Single Object Time Out\n");
				return;
			}
		}
		else
			break;
	}
	*ppBuffer = pBuf->m_Buffer;
	*pLen = pBuf->m_dwBuffLen;
	free(pBuf);
}
void  CAudioGrab::AddTail(char*Buffer, DWORD dwLen)
{
	if (m_dwBufSize > 128 * 1024 * 1024)
		return;

	AudioBuffList*pNewBuf = (AudioBuffList*)malloc(sizeof(AudioBuffList));
	pNewBuf->next = NULL;
	pNewBuf->m_Buffer = (char*)malloc(dwLen);
	pNewBuf->m_dwBuffLen = dwLen;

	memcpy(pNewBuf->m_Buffer, Buffer, dwLen);

	WaitForSingleObject(m_hMutex, INFINITE);

	m_dwBufSize += dwLen;
	if (m_pListHead == NULL)
	{
		m_pListHead = m_pListTail = pNewBuf;
	}
	else
	{
		m_pListTail->next = pNewBuf;
		m_pListTail = pNewBuf;
	}
	SetEvent(m_hEvent);
	SetEvent(m_hMutex);
}