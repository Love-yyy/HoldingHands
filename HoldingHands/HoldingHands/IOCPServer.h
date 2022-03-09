
#pragma once
#include <afx.h>
#include <winsock2.h>
#include <MSWSock.h>

///////////////////////////////////////////////////////////////////////////////////////
//1.对于任意一个socket,Read(或者Write)操作同时只能有一个线程去处理.
//2.每一个完成的IO操作(自定义协议层)，后通知主窗口去处理消息
//3.Send的话只是把数据加到SendList里面,然后判断是否去Post一个completionstatu
//4.每一个客户端，检测远端是否断开，主动关闭，发送数据

///////////////////////////////////////////////////////////////////////////////////////

#define PACKET_CHECK					0x534E4942
#define REMOTE_AND_LOCAL_ADDR_LEN		(2 * (sizeof(sockaddr)+16))
#define IDENTITY_LEN					4

class CPacket;
struct CPacketHeader;
class CClientContext;
class CManager;
//----------------------------------------------------------------

#define IO_ESTABLISHED		1
#define IO_READ				2
#define IO_WRITE			4
#define IO_CLOSE			8

//--------------------------------------------------------

struct OVERLAPPEDPLUS
{
	OVERLAPPED	m_overlapped;
	DWORD		m_IOtype;
	BOOL		m_bManualPost;			//用来记录是否为手动触发。

	OVERLAPPEDPLUS(DWORD IO_TYPE,BOOL bManualPost = FALSE)
	{
		m_IOtype = IO_TYPE;
		m_bManualPost = bManualPost;
		m_overlapped.hEvent = 0;
		m_overlapped.Internal = 0;
		m_overlapped.InternalHigh = 0;
		m_overlapped.Offset = 0;
		m_overlapped.OffsetHigh = 0;
		m_overlapped.Pointer = 0;
	}
};

#define HEART_BEAT		0x00abcdef

//CIOCP只应该有一个实例
class CIOCPServer
{
public:
	static LPFN_ACCEPTEX	lpfnAcceptEx;
private:
	//上传下载速率实现.
	volatile DWORD			m_ReadSpeed;
	volatile DWORD			m_WriteSpeed;
	volatile unsigned int	m_SpeedLock;
	//Listen Socket
	SOCKET			m_ListenSocket;
	//AcceptSocket=================================================================
	HANDLE			m_hStopRunning;								//通知已经停止运行服务器

	volatile SOCKET	m_AcceptSocket;								//用于接收客户端的socket

	char			m_AcceptBuf[REMOTE_AND_LOCAL_ADDR_LEN + IDENTITY_LEN];		//用于储存接收到的客户端的地址
	//==============================================================================
	//Context Manager:
	friend class CClientContext;
	CPtrList*		m_pFreeContextList;
	CPtrList*		m_pClientContextList;						//保存clients
	CRITICAL_SECTION m_csContext;

	//==============================================================================
	//IOCP
	DWORD					m_BusyCount;							//已经获取IO事件的线程数
	DWORD					m_MaxConcurrent;						//IOCP最大并发数量
	DWORD					m_MaxThreadCount;						//线程最多有几个.

	//为了防止所有线程在处理事件的时候等待Write事件造成没有线性去处理Write而形成死锁,需要在线程用尽的时候额外增加线程
	//SendMessage,WaitForSingleObject,GetMessage,SuspendThread会挂起线程.
	//key	ThreadId;
	//value ThreadHandle;
	CMapPtrToPtr*		m_pThreadList;							//ID----Handle
	CRITICAL_SECTION	m_csThread;
	//==============================================================================
	HANDLE			m_hCompletionPort;							//完成端口句柄
	volatile ULONG	m_AssociateClientCount;						//记录和完成端口成功关联的客户端的个数
	//==============================================================================
	friend class	CManager;
	CManager*		m_pManager;
	//
	void UpdateSpeed(DWORD io_type, DWORD TranferredBytes);
	//加到list里面
	void AddToList(CClientContext*pContext);
	//移除
	BOOL RemoveFromList(CClientContext*pContext);
	//有连接了
	void OnAcceptComplete(BOOL bFailed);
	//处理已经完成的IO消息
	void HandlerIOMsg(CClientContext*pClientContext, DWORD nTransferredBytes, DWORD IoType, BOOL bFailed);
	//ReadIO请求完成
	void OnReadComplete(CClientContext*pClientContext, DWORD nTransferredBytes, BOOL bFailed);
	//Write IO请求完成
	void OnWriteComplete(CClientContext*pClientContext, DWORD nTransferredBytes, BOOL bFailed);
	//断开连接
	void OnClose(CClientContext*pClientContext);
	//投递一个Accept请求
	void PostAccept();
	//投递一个ReadIO请求
	void PostRead(CClientContext*pClientContext);
	//投递一个WriteIO请求
	void PostWrite(CClientContext*pClientContext);
	//创建一个ClientContext,断开连接时会自动从list中移除，但是释放需要手动进行
	CClientContext* AllocateContext(SOCKET ClientSocket);
	//要确保在FreeContext调用后不会再做相关的操作
	void FreeContext(CClientContext*pContext);
	//
	static void WorkerThread(void);
	//只允许创建一个实例

	CIOCPServer(HWND hWnd);
	~CIOCPServer();
	//通知主窗口创建相应的Handler
	HWND		m_hNotifyWnd;
	static CIOCPServer* hInstance;
	static void CIOCPServer::async_svr_ctrl_proc(DWORD dwParam);
	
	
public:
	static BOOL SocketInit();						//初始化WSASocket
	static void SocketTerm();						//清理WSASocket
	
#define WM_IOCPSVR_START	(WM_USER+101)
#define WM_IOCPSVR_CLOSE	(WM_USER+102)

	//下面三个函数应该在同一个线程里面执行
	//-------------------------------------------------------------------------------
	DWORD GetWriteSpeed();
	DWORD GetReadSpeed();
	//开启服务器
	BOOL StartServer(USHORT Port);
	//关闭服务器
	void StopServer();
	//由于StartSvr 和 StopSvr需要一些时间,增加异步操作
	void AsyncStartSvr(USHORT Prot);
	//
	void AsyncStopSvr();
	//-------------------------------------------------------------------------------

	static CIOCPServer* CreateServer(HWND hNotifyWnd);
	static void DeleteServer();
};

