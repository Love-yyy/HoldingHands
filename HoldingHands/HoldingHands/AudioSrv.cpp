#include "stdafx.h"
#include "AudioSrv.h"
#include "AudioDlg.h"
#pragma comment(lib,"Winmm.lib")


CAudioSrv::CAudioSrv(DWORD dwIdentity):
CEventHandler(dwIdentity)
{
	//
	m_Idx = 0;
	m_hWaveOut = NULL;
	
	memset(&m_WaveFmt, 0, sizeof(m_WaveFmt));
	m_WaveFmt.wFormatTag = WAVE_FORMAT_PCM; // ACM will auto convert wave format
	m_WaveFmt.nChannels = 1;
	m_WaveFmt.nSamplesPerSec = 44100;
	m_WaveFmt.nAvgBytesPerSec = 44100 * 2;
	m_WaveFmt.nBlockAlign = 2;
	m_WaveFmt.wBitsPerSample = 16;
	m_WaveFmt.cbSize = 0;					//额外信息
	//
	m_pDlg = NULL;
}


CAudioSrv::~CAudioSrv()
{
}

void CAudioSrv::OnConnect()
{
	m_pDlg = new CAudioDlg(this);
	if (FALSE == m_pDlg->Create(IDD_AUDIODLG))
	{
		Disconnect();
		return;
	}
	m_pDlg->ShowWindow(SW_SHOW);
	//开始
	Send(AUDIO_BEGIN, 0, 0);
}
void CAudioSrv::OnClose()
{
	if (m_pDlg)
	{
		delete m_pDlg;
		m_pDlg = NULL;
	}
	AudioOutTerm();
}

BOOL CAudioSrv::AudioOutInit()
{
	MMRESULT mmResult = 0;
	//打开设备
	mmResult = waveOutOpen(&m_hWaveOut, WAVE_MAPPER, &m_WaveFmt, NULL, 0, CALLBACK_NULL);
	if (mmResult != MMSYSERR_NOERROR)
		return FALSE;

	memset(m_hdrs, 0, sizeof(m_hdrs));
	//
	for (int i = 0; i < 16; i++)
	{
		m_hdrs[i].lpData = buffs[i];
		m_hdrs[i].dwBufferLength = LEN_PER_BUFF;
	}
	return TRUE;
}
void CAudioSrv::AudioOutTerm()
{
	//
	if (m_hWaveOut)
	{
		waveOutPause(m_hWaveOut);
		waveOutClose(m_hWaveOut);
		m_hWaveOut = NULL;
	}
	//
	memset(m_hdrs, 0, sizeof(m_hdrs));
	m_Idx = 0;
}

void CAudioSrv::OnAudioData(char*Buffer, DWORD dwLen)
{
	MMRESULT mmResult = 0;
	if (m_hWaveOut == NULL)
	{
		if (FALSE == AudioOutInit())
		{
			WCHAR szError[] = L"AudioOutInit Failed!";
			m_pDlg->SendMessage(WM_AUDIO_ERROR, (WPARAM)szError, 0);
			return;
		}
	}
	m_hdrs[m_Idx].dwBytesRecorded = 0;
	m_hdrs[m_Idx].dwFlags = 0;
	m_hdrs[m_Idx].dwLoops = 0;
	m_hdrs[m_Idx].dwUser = 0;
	m_hdrs[m_Idx].lpNext = 0;
	m_hdrs[m_Idx].reserved = 0;

	m_hdrs[m_Idx].dwBufferLength = dwLen;

	if (dwLen > LEN_PER_BUFF)
	{
		WCHAR szError[] = L"Buffer Overflow!";
		m_pDlg->SendMessage(WM_AUDIO_ERROR, (WPARAM)szError, 0);
		return;
	}
	//拷贝buffer,
	memcpy(m_hdrs[m_Idx].lpData, Buffer, dwLen);

	mmResult = waveOutPrepareHeader(m_hWaveOut, &m_hdrs[m_Idx], sizeof(WAVEHDR));
	if (mmResult != MMSYSERR_NOERROR)
	{
		WCHAR szError[] = L"waveOutPrepareHeader Failed!";
		m_pDlg->SendMessage(WM_AUDIO_ERROR, (WPARAM)szError, 0);
		return;
	}
	mmResult = waveOutWrite(m_hWaveOut, &m_hdrs[m_Idx], sizeof(WAVEHDR));
	if (mmResult != MMSYSERR_NOERROR)
	{
		WCHAR szError[] = L"waveOutWrite Failed!";
		m_pDlg->SendMessage(WM_AUDIO_ERROR, (WPARAM)szError, 0);
		return;
	}
	m_Idx = (m_Idx + 1) % BUFF_COUNT;
}
void CAudioSrv::OnReadComplete(WORD Event, DWORD Total, DWORD dwRead, char*Buffer)
{
	switch (Event)
	{
	case AUDIO_DATA:
		OnAudioData(Buffer, dwRead);
		break;
	case AUDIO_ERROR:
		OnAudioError((WCHAR*)Buffer);
		break;
	default:
		break;
	}
}

void CAudioSrv::OnAudioError(WCHAR*szError)
{
	m_pDlg->SendMessage(WM_AUDIO_ERROR, (WPARAM)szError, 0);
}