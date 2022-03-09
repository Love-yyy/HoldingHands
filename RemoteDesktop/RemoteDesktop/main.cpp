#include "IOCPClient.h"
#include "RemoteDesktop.h"

#pragma comment(lib,"libx264.lib")
#pragma comment(lib,"yuv.lib")


extern "C"  __declspec(dllexport) void HHBeginDesktop(char* szServerAddr, unsigned short uPort, DWORD dwParam);


void HHBeginDesktop(char* szServerAddr, unsigned short uPort, DWORD dwParam)
{
	CIOCPClient::SocketInit();

	CIOCPClient client(szServerAddr, uPort);

	CRemoteDesktop rd(REMOTEDESKTOP);

	client.BindHandler(&rd);

	client.Run();

	client.UnbindHandler();

	CIOCPClient::SocketTerm();
}

//int main()
//{
//	//HHBeginDesktop("81.68.224.152", 10086, 0);
//	HHBeginDesktop("10.148.169.252", 10086, 0);
//	system("pause");
//}