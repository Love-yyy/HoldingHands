#pragma once
#include "EventHandler.h"
#include<mmreg.h>

#define BUFF_COUNT 16
#define LEN_PER_BUFF 0x2000

class CAudioGrab
{
	struct AudioBuffList
	{
		char*	m_Buffer;
		DWORD	m_dwBuffLen;
		struct AudioBuffList*next;
	};

private:
	AudioBuffList*m_pListHead;
	AudioBuffList*m_pListTail;
	DWORD		  m_dwBufSize;
	HANDLE		  m_hEvent;
	HANDLE		  m_hMutex;

	HWAVEIN		  m_hWaveIn;		//输入设备

	volatile long m_bStop;


	WAVEFORMATEX	 m_Wavefmt;	//格式
	
	void  RemoveHead(char**ppBuffer, DWORD*pLen);
	void  AddTail(char*Buffer, DWORD dwLen);

	WAVEHDR		m_hdrs[BUFF_COUNT];
	char		m_Buffers[BUFF_COUNT][LEN_PER_BUFF];	//
	UINT		m_Idx;

	HANDLE		m_hWorkThread;
	DWORD		m_dwThreadID;
	static void __stdcall WorkThread(CAudioGrab*pThis);
	
public:
	BOOL	GrabInit();
	void	GrabTerm();

	BOOL	StartGrab();
	void	StopGrab();

	BOOL	GetBuffer(void**ppBuffer,DWORD*pBufLen);

	CAudioGrab();
	~CAudioGrab();
};

