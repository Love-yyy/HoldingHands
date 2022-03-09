#include "IOCPClient.h"
#include "Chat.h"

/****************************Chat Module Entry *********************************/

extern "C" __declspec(dllexport) void HHBeginChat(char* szServerAddr,unsigned short uPort,DWORD dwParam);

void HHBeginChat(char* szServerAddr,unsigned short uPort,DWORD dwParam)
{

	CIOCPClient Client(szServerAddr,uPort);
	CChat chat(CHAT);

	Client.BindHandler(&chat);
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
	HHBeginChat("81.68.224.152", 10086, 0);
	CIOCPClient::SocketTerm();
	return 0;
}
#endif