#pragma once
#include "EventHandler.h"

#define CMD				('C'|('M')<<8|('D')<<16)

//0³É¹¦,-1Ê§°Ü
#define CMD_BEGIN		(0xcad0)

#define CMD_COMMAND		(0xcad1)
#define CMD_RESULT		(0xcad2)

class CCmd :
	public CEventHandler
{
public:
	HANDLE		m_hReadThread;		

	HANDLE		m_hReadPipe;
	HANDLE		m_hWritePipe;
	
	HANDLE		m_hCmdProcess;

	PROCESS_INFORMATION m_pi;
	void OnClose();		
	void OnConnect();

	void OnReadPartial(WORD Event, DWORD Total, DWORD Read, char*Buffer);
	void OnReadComplete(WORD Event, DWORD Total, DWORD Read, char*Buffer);

	int CmdBegin();

	void OnCommand(char*szCmd);
	CCmd(DWORD dwIdentity);
	~CCmd();
};

