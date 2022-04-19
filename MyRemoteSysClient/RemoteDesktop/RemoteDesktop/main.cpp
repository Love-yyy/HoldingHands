#include "IOCPClient.h"
#include "RemoteDesktop.h"

#pragma comment(lib,"libx264.lib")
#pragma comment(lib,"yuv.lib")


extern "C"  __declspec(dllexport) void HHBeginDesktop(char* szServerAddr, unsigned short uPort, DWORD dwParam);


void HHBeginDesktop(char* szServerAddr, unsigned short uPort, DWORD dwParam)
{
	CIOCPClient client(szServerAddr, uPort);

	CRemoteDesktop rd(REMOTEDESKTOP);

	client.BindHandler(&rd);

	client.Run();

	client.UnbindHandler();

}

#ifdef _DEBUG
int main()
{
	CIOCPClient::SocketInit();
	HHBeginDesktop("192.168.1.235", 10086, 0);
	CIOCPClient::SocketTerm();
}
#endif
