#pragma once
#include "EventHandler.h"

#include<mmreg.h>
#include<mmeapi.h>

#define AUDIO		('A'|'U'<<8|'D'<<16|'O'<<24)
#define AUDIO_ERROR (0x0000)

#define AUDIO_BEGIN (0xaaa0)
#define AUDIO_DATA	(0xaaa1)
#define AUDIO_STOP  (0xaaa2)

#define BUFF_COUNT 16
#define LEN_PER_BUFF 0x2000


class CAudioDlg;
class CAudioSrv :
	public CEventHandler
{
private:
	WAVEHDR	 m_hdrs[BUFF_COUNT];
	char	 buffs[BUFF_COUNT][LEN_PER_BUFF];
	HWAVEOUT m_hWaveOut;
	WAVEFORMATEX m_WaveFmt;	//¸ñÊ½
	DWORD	 m_Idx;

	CAudioDlg*	m_pDlg;
	BOOL AudioOutInit();
	void AudioOutTerm();
public:
	void OnConnect();
	void OnClose();
	void OnAudioData(char*Buffer,DWORD dwLen);

	void OnAudioError(WCHAR*szError);
	void OnReadComplete(WORD Event, DWORD Total, DWORD dwRead, char*Buffer);
	CAudioSrv(DWORD dwIdentity);
	~CAudioSrv();
};

