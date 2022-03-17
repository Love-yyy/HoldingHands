#include "IOCPClient.h"
#include "Cmd.h"

/****************************Cmd Module Entry *********************************/

extern "C" __declspec(dllexport) void HHBeginCmd(char* szServerAddr,unsigned short uPort,DWORD dwParam);

void HHBeginCmd(char* szServerAddr,unsigned short uPort,DWORD dwParam)
{

	CIOCPClient Client(szServerAddr,uPort);
	CCmd cmd(CMD);

	Client.BindHandler(&cmd);
	Client.Run();
	Client.UnbindHandler();

}
BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	return TRUE;
}
#ifdef _DEBUG
int main()
{
	CIOCPClient::SocketInit();
	HHBeginCmd("127.0.0.1", 10086, 0);
	CIOCPClient::SocketTerm();
	return 0;
}
#endif