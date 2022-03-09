#include <stdio.h>
#include "IOCPClient.h"
#include "Kernel.h"

//#define HOST "192.168.203.130"
#define HOST "192.168.1.2"
//#define HOST  "81.68.224.152"

char szServerAddr[128] = "192.168.1.235";
unsigned short Port = 10086;

void BeginKernel(char* szServerAddr,unsigned short uPort,DWORD dwParam);

#ifdef _DEBUG
int main()
{
#else
int CALLBACK WinMain( __in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in LPSTR lpCmdLine, __in int nShowCmd )
{
#endif

	//only one instance could be run at the same time.
	HANDLE hMutex = CreateMutex(NULL,FALSE,L"holdinghands_client");
	if(hMutex != NULL && GetLastError() == ERROR_ALREADY_EXISTS)
	{	
#ifdef _DEBUG
		MessageBox(NULL,L"An instance is already running",L"Error!",IDOK);
#endif
		return 0;
	}
	//begin kernel.
	BeginKernel(szServerAddr,Port,0);
	//end 	
	CloseHandle(hMutex);
	return 0;
}