#include "IOCPClient.h"
#include "Packet.h"
#include "EventHandler.h"

#pragma comment(lib,"ws2_32.lib")

CIOCPClient::CIOCPClient(const char*ServerAddr, int ServerPort, BOOL bReConnect, UINT MaxTryCount, UINT ReconnectInterval)
{
	//自动重新连接
	m_isConnecting = FALSE;
	m_bReConnect = bReConnect;
	m_MaxTryCount = MaxTryCount;
	m_ReconnectInterval = ReconnectInterval;
	//服务器信息
	strcpy(m_ServerAddr, ServerAddr);
	m_ServerPort = ServerPort;
	//
	m_ClientSocket = INVALID_SOCKET;
	//
	m_dwRead = 0;
	m_wsaReadBuf.buf = 0;
	m_wsaReadBuf.len = 0;
	//
	m_SendPacketOver = CreateEvent(NULL, FALSE, TRUE, NULL);
	//
	m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 2);

	m_pHandler = 0;
	InitializeCriticalSectionAndSpinCount(&m_csCheck, 4000);
}

CIOCPClient::~CIOCPClient()
{
	//
	if (m_ClientSocket != INVALID_SOCKET)
	{
		closesocket(m_ClientSocket);
		m_ClientSocket = INVALID_SOCKET;
	}
	//
	//
	if (m_SendPacketOver != NULL)
	{
		CloseHandle(m_SendPacketOver);
		m_SendPacketOver = NULL;
	}
	//
	if (m_hCompletionPort != NULL)
	{
		CloseHandle(m_hCompletionPort);
		m_hCompletionPort = NULL;
	}
	//
	if (m_pHandler)
	{
		//最好不要让这里的代码执行
		delete m_pHandler;
		m_pHandler = NULL;
	}
	DeleteCriticalSection(&m_csCheck);
}

BOOL CIOCPClient::BindHandler(CEventHandler*pHandler)
{
	if (m_pHandler != NULL)
		return FALSE;
	m_pHandler =  pHandler;
	m_pHandler->m_pClient = this;
	return TRUE;
}

BOOL CIOCPClient::UnbindHandler()
{
	if (m_pHandler == NULL)
		return FALSE;
	m_pHandler->m_pClient = NULL;
	m_pHandler = NULL;
	return TRUE;
}
void CIOCPClient::Run()
{
	if (m_hCompletionPort == NULL || m_pHandler == NULL || m_SendPacketOver == NULL)
	{
		return;
	}
	IocpRun();
}

BOOL CIOCPClient::SocketInit()
{
	WSADATA			wsadata = { 0 };
	WORD			RequestedVersion = MAKEWORD(2, 2);
	//初始化WinSock
	if (SOCKET_ERROR == WSAStartup(RequestedVersion, &wsadata))
	{
		wprintf(L"WSAStartup failed with error: %u\n", WSAGetLastError());
		return FALSE;
	}
	return TRUE;
}

void CIOCPClient::SocketTerm()
{
	WSACleanup();
}

void CIOCPClient::Disconnect()
{
	EnterCriticalSection(&m_csCheck);
	if (m_ClientSocket != INVALID_SOCKET)
	{
		closesocket(m_ClientSocket);
		m_ClientSocket = INVALID_SOCKET;
	}
	LeaveCriticalSection(&m_csCheck);
}
//连接服务器
BOOL CIOCPClient::TryConnect()
{
	INT				iResult = 0;
	LPFN_CONNECTEX	lpfnConnectEx = NULL;
	GUID			GuidConnectEx = WSAID_CONNECTEX;
	DWORD			dwBytes;
	SOCKET			temp;
	//先获取ConnectEx指针
	if (lpfnConnectEx == NULL)
	{
		iResult = INVALID_SOCKET;
		temp = socket(AF_INET, SOCK_STREAM, NULL);
		if (temp != INVALID_SOCKET)
		{
			iResult = WSAIoctl(temp, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidConnectEx, sizeof(GuidConnectEx),
				&lpfnConnectEx, sizeof(lpfnConnectEx), &dwBytes, NULL, NULL);
			closesocket(temp);

			if (iResult == SOCKET_ERROR)
			{
				printf("WSAIoctl failed with error: %u\n", WSAGetLastError());
				WSACleanup();
				return FALSE;
			}
		}
	}
	if (lpfnConnectEx == NULL)
		return FALSE;
	//上一次连接失败后,不会关闭socket,可以继续使用
	if (m_ClientSocket == INVALID_SOCKET)
	{
		m_ClientSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (m_ClientSocket == INVALID_SOCKET)
			return FALSE;
		//绑定本地
		SOCKADDR_IN local;
		local.sin_family = AF_INET;
		local.sin_addr.S_un.S_addr = INADDR_ANY;
		local.sin_port = 0;
		if (SOCKET_ERROR == bind(m_ClientSocket, (LPSOCKADDR)&local, sizeof(local)))
		{
			closesocket(m_ClientSocket);
			m_ClientSocket = INVALID_SOCKET;
			return FALSE;
		}
		//把这个socket 绑定到完成端口
		if (m_hCompletionPort != CreateIoCompletionPort((HANDLE)m_ClientSocket, m_hCompletionPort, NULL, 0))
		{
			closesocket(m_ClientSocket);
			m_ClientSocket = INVALID_SOCKET;
			return FALSE;
		}//
	}
	//获取目标主机ip
	HOSTENT*pHost = gethostbyname(m_ServerAddr);
	SOCKADDR_IN ServerAddr = { 0 };
	ServerAddr.sin_family = AF_INET;
	memcpy(&ServerAddr.sin_addr.S_un.S_addr, pHost->h_addr, 4);
	ServerAddr.sin_port = htons(m_ServerPort);
	//标记正在连接
	m_isConnecting = TRUE;
	//-------------------------Post Connect------------------------ 
	OVERLAPPEDPLUS*pOverlapped = new OVERLAPPEDPLUS(IO_CONNECT);
	//int ret = connect(m_ClientSocket, (sockaddr*)&ServerAddr, sizeof(ServerAddr));
	BOOL bResult = lpfnConnectEx(m_ClientSocket, (sockaddr*)&ServerAddr, sizeof(ServerAddr),&m_pHandler->m_Identity,4,NULL, (OVERLAPPED*)pOverlapped);
	int Error = WSAGetLastError();
	if (FALSE == bResult && Error != WSA_IO_PENDING)
	{
#ifdef _DEBUG
		printf("SOCKET :%u\n", m_ClientSocket);
		printf("WSAGetLastError : %d \n", WSAGetLastError());
#endif
		pOverlapped->m_bManualPost = TRUE;
		PostQueuedCompletionStatus(m_hCompletionPort, 0, 0, (OVERLAPPED*)pOverlapped);
	}
	return TRUE;
}

void CIOCPClient::WorkThread(CIOCPClient*pClient)
{
	DWORD dwTransferBytes;
	ULONG ulCompletionKey;
	OVERLAPPEDPLUS *pOverlapped;
	BOOL bResult;
	BOOL bEOF;
	BOOL bLoop = TRUE;
	HANDLE hCompletionPort = pClient->m_hCompletionPort;
	
	while (bLoop)
	{
		for (;;)
		{
			dwTransferBytes = 0;
			ulCompletionKey = 0;
			pOverlapped = NULL;
			bResult = FALSE;
			bEOF = FALSE;
			bResult = GetQueuedCompletionStatus(hCompletionPort, &dwTransferBytes, &ulCompletionKey, (LPOVERLAPPED*)&pOverlapped, INFINITE);
			//退出线程.
			if (ulCompletionKey == INVALID_SOCKET)
			{
				bLoop = FALSE;
				break;
			}
			//-------------------------------处理IO消息-----------------------------
			bEOF = ((!bResult) || (!dwTransferBytes) || pOverlapped->m_bManualPost);

			if (pOverlapped->m_IOtype == IO_CONNECT)
			{

				int seconds = 0;
				int bytes = sizeof(seconds);
				int iResult = 0;
				if (!bResult || pOverlapped->m_bManualPost || dwTransferBytes != 4) {		//ConnectEx失败
					break;
				}
				//Update properties;
				setsockopt(pClient->m_ClientSocket, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
				//-------------------------------
				pClient->OnConnect();
				//-------------------------------
				//begin recv data:
				pClient->m_wsaReadBuf.buf = pClient->m_ReadPacket.GetBuffer();
				pClient->m_wsaReadBuf.len = PACKET_HEADER_LEN;
				pClient->m_dwRead = 0;
				//Post first read,then the socket will continuously process the read completion statu.;
				pClient->PostRead();
			}
			else if (pOverlapped->m_IOtype == IO_READ){
				pClient->OnReadComplete(dwTransferBytes, bEOF);
				//断开连接了.(只根据IO_READ判断.)
				if (bEOF){
					pClient->Disconnect();
					pClient->OnClose();
					break;
				}
			}
			else{
				pClient->OnWriteComplete(dwTransferBytes, bEOF);
			}

			if (pOverlapped) delete pOverlapped;
		}
		//不退出线程.
		if (bLoop)
		{
			//连接失败或者断开连接会执行这里
			if (pClient->m_isConnecting)
				pClient->m_FailedCount++;

			//if we will reconnect server.
			if (pClient->m_bReConnect == FALSE || pClient->m_FailedCount >= pClient->m_MaxTryCount){
				//投递两个消息,退出.
				for (int i = 0; i < 2; i++){
					PostQueuedCompletionStatus(pClient->m_hCompletionPort, 0,INVALID_SOCKET,0 );
				}
			}
			else{
				Sleep(1000 * pClient->m_ReconnectInterval);
				//重新发起连接.
				pClient->TryConnect();
			}
		}
	}
}

void CIOCPClient::IocpRun()
{
	//一个用于IOCP 的读处理，一个用于写处理,(只用一个线程会造成死锁,Send会等待上一个发完,而必须线程处理了才能知道发完没)
	for (int i = 0; i < 2; i++)
		m_hWorkThreads[i] = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)(WorkThread), (LPVOID)this, 0, 0);

	//失败,直接退出.
	if (FALSE == TryConnect())
	{
		for (int i = 0; i < 2;i++)
			PostQueuedCompletionStatus(m_hCompletionPort, 0, (ULONG_PTR)INVALID_SOCKET, 0);
	}

	while (1){
		DWORD dwResult = WaitForMultipleObjects(2, m_hWorkThreads, TRUE, 20000);
		if (dwResult == WAIT_OBJECT_0)
			break;
		//超时了.
#ifdef _DEBUG
		printf("Time out");
#endif
		if (m_ClientSocket != INVALID_SOCKET && !m_isConnecting){
			BOOL bHeartBeatReply = InterlockedExchange((volatile unsigned int*)&m_bHeartBeatReply, FALSE);
			if (!bHeartBeatReply){
#ifndef _DEBUG
				printf("Not Recv Heart Beat Repely!\n Disonnect\nBye!");
#endif
				Disconnect();
			}
			else{
#ifdef _DEBUG
				printf("Send Heart Beat\n");
#endif
				SendPacket(HEART_BEAT, 0, 0);
			}
		}
#ifdef _DEBUG
		else printf("Don't Send Heart Beat\n");
#endif
	}

	//清理工作
	for (int i = 0; i < 2; i++)
		CloseHandle(m_hWorkThreads[i]);
}

//发送包.
void CIOCPClient::SendPacket(DWORD command, char*data, int len)
{
	//等待上一个包发送完毕.
	for (;;)
	{
		DWORD dwResult = WaitForSingleObject(m_SendPacketOver, 500);
		if (dwResult == WAIT_TIMEOUT && m_ClientSocket == INVALID_SOCKET)
			return;
		if (dwResult == WAIT_OBJECT_0)
			break;
	}
	
	if (FALSE == m_WritePacket.AllocateMem(len))
	{
		SetEvent(m_SendPacketOver);//发送失败
		return ;
	}
	//Set Header
	m_WritePacket.SetHeader(len, command);
	//Copy Data.
	memcpy(m_WritePacket.GetBody(), data, len);
	//发送Header:
	m_dwWrite = 0;
	m_wsaWriteBuf.buf = m_WritePacket.GetBuffer();
	m_wsaWriteBuf.len = m_WritePacket.GetBodyLen() + PACKET_HEADER_LEN;

	PostWrite();
}


void CIOCPClient::OnWriteComplete(DWORD nTransferredBytes, BOOL bEOF)
{
	if (!bEOF){
		m_dwWrite += nTransferredBytes;
		//发完心跳包忽略处理.
		if (m_dwWrite == m_WritePacket.GetBodyLen() + PACKET_HEADER_LEN){
			if (m_WritePacket.GetCommand()!=HEART_BEAT)
				m_pHandler->OnWriteComplete(m_WritePacket.GetCommand(), m_WritePacket.GetBodyLen(), m_dwWrite, m_WritePacket.GetBody());
		}
		else if (m_dwWrite < m_WritePacket.GetBodyLen() + PACKET_HEADER_LEN){
			if (m_dwWrite > PACKET_HEADER_LEN && m_WritePacket.GetCommand() != HEART_BEAT){
				m_pHandler->OnWritePartial(m_WritePacket.GetCommand(), m_WritePacket.GetBodyLen(), m_dwWrite, m_WritePacket.GetBody());
			}
			m_wsaWriteBuf.buf = m_WritePacket.GetBuffer() + m_dwWrite;
			m_wsaWriteBuf.len = m_WritePacket.GetBodyLen() + PACKET_HEADER_LEN - m_dwWrite;
			//continue send data
			PostWrite();
			return;
		}
	}
	m_dwWrite = 0;
	m_wsaWriteBuf.buf = 0;
	m_wsaWriteBuf.len = 0;
	SetEvent(m_SendPacketOver);
}
//ReadIO请求完成
void CIOCPClient::OnReadComplete(DWORD nTransferredBytes, BOOL bEOF)
{
	if (bEOF)
	{
		m_dwRead = 0;
		return;
	}
	//成功,有数据到达：
	m_dwRead += nTransferredBytes;
	//读取完毕
	if (m_dwRead >= PACKET_HEADER_LEN
		&& (m_dwRead == PACKET_HEADER_LEN + m_ReadPacket.GetBodyLen()))
	{
		//处理完成的包
		if (m_ReadPacket.GetCommand() == HEART_BEAT){
#ifdef _DEBUG
			printf("Recv Heart Beat\n");
#endif
			m_bHeartBeatReply = TRUE;
		}
		else{
#ifdef _DEBUG
			printf("%x\n",m_ReadPacket.GetCommand());
#endif
			WORD Event = m_ReadPacket.GetCommand();
			DWORD dwTotal = m_ReadPacket.GetBodyLen();						//buffer总长度
			DWORD dwRead = m_dwRead - PACKET_HEADER_LEN;					//已经读取的长度
			m_pHandler->OnReadComplete(Event, dwTotal, dwRead, m_ReadPacket.GetBody());
		}
		m_dwRead = 0;
	}
	//先把PacketHeader接收完
	if (m_dwRead < PACKET_HEADER_LEN)
	{
		m_wsaReadBuf.buf = m_ReadPacket.GetBuffer();
		m_wsaReadBuf.len = PACKET_HEADER_LEN - m_dwRead;
	}
	else
	{
		BOOL bOk = m_ReadPacket.Verify();
		BOOL bMemEnough = TRUE;
		//为空，先分配相应内存,在这之前先校验头部是否正确
		if (bOk)
		{
			bMemEnough = m_ReadPacket.AllocateMem(m_ReadPacket.GetBodyLen());
		}
		if (bOk && m_dwRead > PACKET_HEADER_LEN)
		{
			//处理完成的包
			WORD Event = m_ReadPacket.GetCommand();
			DWORD dwTotal = m_ReadPacket.GetBodyLen();
			DWORD dwRead = m_dwRead - PACKET_HEADER_LEN;
			m_pHandler->OnReadPartial(Event, dwTotal, dwRead, m_ReadPacket.GetBody());
		}
		//如果bOk且，内存分配成功，最后会重新赋值，否则下一次Post会断开这个客户的连接
		m_wsaReadBuf.buf = NULL;
		m_wsaReadBuf.len = 0;
		//内存可能分配失败
		if (bOk && bMemEnough)
		{
			//设置偏移
			m_wsaReadBuf.buf = m_ReadPacket.GetBuffer() + m_dwRead;
			//计算剩余长度
			m_wsaReadBuf.len = m_ReadPacket.GetBodyLen() + PACKET_HEADER_LEN - m_dwRead;
		}
	}
	//投递下一个请求
	PostRead();
}
//投递一个ReadIO请求
void CIOCPClient::PostRead()
{
	DWORD nReadBytes = 0;
	DWORD flag = 0;
	int nIORet, nErrorCode;
	OVERLAPPEDPLUS*pOverlapped = new OVERLAPPEDPLUS(IO_READ);
	EnterCriticalSection(&m_csCheck);
	//assert(pClientContext->m_ClientSocket == INVALID_SOCKET);
	nIORet = WSARecv(m_ClientSocket, &m_wsaReadBuf, 1, &nReadBytes, &flag, (LPOVERLAPPED)pOverlapped, NULL);
	nErrorCode = WSAGetLastError();
	//IO错误,该SOCKET被关闭了,或者buff为NULL;
	if (nIORet == SOCKET_ERROR	&& nErrorCode != WSA_IO_PENDING)
	{
		//将这个标记为手动投递的
		pOverlapped->m_bManualPost = TRUE;
		PostQueuedCompletionStatus(m_hCompletionPort, 0, 0, (LPOVERLAPPED)pOverlapped);
	}
	LeaveCriticalSection(&m_csCheck);
}

void CIOCPClient::PostWrite()
{
	DWORD nWriteBytes = 0;
	DWORD flag = 0;
	int nIORet, nErrorCode;
	OVERLAPPEDPLUS*pOverlapped = new OVERLAPPEDPLUS(IO_WRITE);

	EnterCriticalSection(&m_csCheck);

	nIORet = WSASend(m_ClientSocket, &m_wsaWriteBuf, 1, &nWriteBytes, flag, (LPOVERLAPPED)pOverlapped, NULL);
	nErrorCode = WSAGetLastError();
	//IO错误,该SOCKET被关闭了,或者buff为NULL;
	if (nIORet == SOCKET_ERROR	&& nErrorCode != WSA_IO_PENDING)
	{
		pOverlapped->m_bManualPost = TRUE;
		PostQueuedCompletionStatus(m_hCompletionPort, 0, 0, (LPOVERLAPPED)pOverlapped);
	}
	LeaveCriticalSection(&m_csCheck);
}
void CIOCPClient::OnClose()
{
	//等待Write完成.
	WaitForSingleObject(m_SendPacketOver, INFINITE);
	m_pHandler->OnClose();
}

void CIOCPClient::OnConnect()
{
	m_FailedCount = 0;				
	m_isConnecting = FALSE;
	//设置初始值为TRUE;
	InterlockedExchange((volatile unsigned int*)&m_bHeartBeatReply, TRUE);
	SetEvent(m_SendPacketOver);	

	//保存sockaddr和peer addr
	m_PeerPort = 0;
	m_SockPort = 0;

	m_szPeerAddr[0] = 0;
	m_szSockAddr[0] = 0;

	sockaddr_in addr = { 0 };
	int namelen = sizeof(addr);
	//本地
	getsockname(m_ClientSocket, (sockaddr*)&addr, &namelen);
	strcpy(m_szSockAddr, inet_ntoa(addr.sin_addr));
	m_SockPort = addr.sin_port;
	//对方
	namelen = sizeof(addr);
	getpeername(m_ClientSocket, (sockaddr*)&addr, &namelen);
	strcpy(m_szPeerAddr, inet_ntoa(addr.sin_addr));
	m_PeerPort = addr.sin_port;
	//交给Handler处理。
	m_pHandler->OnConnect();
}

void CIOCPClient::GetPeerName(char* Addr, USHORT&Port)
{
	Port = m_PeerPort;
	strcpy(Addr, m_szPeerAddr);
}
void CIOCPClient::GetSockName(char* Addr, USHORT&Port)
{
	Port = m_SockPort;
	strcpy(Addr, m_szSockAddr);
}

void CIOCPClient::GetSrvName(char*Addr, USHORT&Port)
{
	Port = m_ServerPort;
	strcpy(Addr, m_ServerAddr);
}