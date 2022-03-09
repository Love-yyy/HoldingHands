#pragma once
#include "EventHandler.h"

#define FILE_MANAGER	('F'|('M'<<8)|('G')<<16|('R'<<24))
#define FILE_MGR_CHDIR			0x00a1
#define FILE_MGR_CHDIR_RET		0x00a2
//NewDir.
#define FILE_MGR_GETLIST		0x00a3
#define FILE_MGR_GETLIST_RET	0x00a4

#define FILE_MGR_UP				0x00a5
#define FILE_MGR_REFRESH		0x01a6
#define FILE_MGR_SEARCH			0x01a7	



#define FILE_MGR_UPLOADFROMDISK	0x00a6
#define FILE_MGR_UPLOADFRURL	0x00a7

#define FILE_MGR_DOWNLOAD		0x00a8

#define FILE_MGR_RUNFILE_NORMAL	0x00a9
#define FILE_MGR_RUNFILE_HIDE	0x00aa
//
#define FILE_MGR_NEWFOLDER		0x00ab
#define FILE_MGR_RENAME			0x00ac
#define FILE_MGR_DELETE			0x00ad
#define FILE_MGR_COPY			0x00ae
#define FILE_MGR_CUT			0x00af
#define FILE_MGR_PASTE			0x00b0

#define FILE_MGR_PREV_UPLOADFROMDISK	0x1101
#define FILE_MGR_PREV_UPLOADFRURL		0x1102
#define FILE_MGR_PREV_DOWNLOAD			0x1103

#define FILE_MGR_PREV_NEWFOLDER			0x1104
#define FILE_MGR_PREV_RENAME			0x1105



class CFileManagerDlg;

class CFileManagerSrv :
	public CEventHandler
{
public:
	CFileManagerDlg *m_pDlg;

	void OnClose();					//当socket断开的时候调用这个函数
	void OnConnect();				//当socket连接的时候调用这个函数
	//有数据到达的时候调用这两个函数.
	void OnReadPartial(WORD Event, DWORD Total, DWORD nRead, char*Buffer);
	void OnReadComplete(WORD Event, DWORD Total, DWORD nRead, char*Buffer);
	//有数据发送完毕后调用这两个函数
	void OnWritePartial(WORD Event, DWORD Total, DWORD nWrite, char*Buffer);
	void OnWriteComplete(WORD Event, DWORD Total, DWORD nWrite, char*Buffer);


	void OnChangeDirRet(DWORD dwRead,char*buffer);
	void OnGetListRet(DWORD dwRead, char*buffer);


	void Up();
	void Refresh();
	void NewFolder(WCHAR*szName);
	void Rename(WCHAR*szNames);
	void ChDir(WCHAR*szNewDir);
	void Search();

	void Delete(WCHAR*szFileList);
	void Copy(WCHAR*szFileList);
	void Cut(WCHAR*szFileList);
	void Paste();

	void PrevUploadFromUrl();
	void PrevUploadFromDisk();
	void PrevDownload();
	void PrevRename();
	void PrevNewFolder();

	void UploadFromUrl(WCHAR*szUrl);
	void UploadFromDisk(WCHAR*szFileList);
	void Download(WCHAR*szFileList);
	void RunFileNormal(WCHAR*szFileList);
	void RunFileHide(WCHAR*szFileList);
	CFileManagerSrv(DWORD dwIdentity);
	~CFileManagerSrv();
};

