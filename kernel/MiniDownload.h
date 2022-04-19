#pragma once
#include "EventHandler.h"
#include <WinInet.h>
#define MINIDOWNLOAD	('M'|('N'<<8)|('D'<<16)|('D'<<24))

//OnConnect
#define MNDD_STATU_ANALYSE_URL_FAILED		(0x1)
#define MNDD_STATU_INTERNETOPEN_FAILED		(0x2)
#define MNDD_STATU_INTERNETCONNECT_FAILED	(0x3)
#define MNDD_STATU_OPENREMOTEFILE_FILED		(0x4)

#define MNDD_STATU_UNKNOWN_FILE_SIZE		(0x5)	//文件大小未知;
#define MNDD_STATU_UNSUPPORTED_PROTOCOL		(0x6)
#define MNDD_STATU_HTTPSENDREQ_FAILED		(0x7)

#define MNDD_STATU_OPENLOCALFILE_FAILED		(0x8)

#define MNDD_GET_FILE_INFO		(0xabc0)
#define MNDD_FILE_SIZE			(0xabd1)

#define MNDD_DOWNLOAD_CONTINUE	(0xabd3)

//成功,创建文件失败.;
#define MNDD_DOWNLOAD_RESULT	(0xabd4)

#define MNDD_DOWNLOAD_END		(0xabd6)


class CMiniDownload :
	public CEventHandler
{
	typedef struct {
		DWORD dwStatu;
		DWORD dwFileSizeLo;
		DWORD dwFileSizeHi;
	}MnddFileInfo;

	/*
		0.Ok
		1.InternetReadFileFailed.
		2.WriteFileFailed.
		3.Finished.
	*/
	typedef struct
	{
		DWORD dwStatu;			
		DWORD dwWriteSize;
	}DownloadResult;

private:
	HINTERNET	m_hInternet;
	HINTERNET	m_hConnect;
	HINTERNET	m_hRemoteFile;
	HANDLE		m_hLocalFile;

	ULONGLONG	m_ullTotalSize;
	ULONGLONG	m_ullFinishedSize;

	URL_COMPONENTSW url;

	char*	 m_Buffer;
	WCHAR*	 m_pInit;

	WCHAR*	 m_szSavePath;
	WCHAR*	 m_szUrlPath;
	WCHAR*	 m_szHost;
	WCHAR*	 m_szUser;
	WCHAR*	 m_szPassword;
	WCHAR*	 m_szExtraInfo;

public:
	void OnClose();		
	void OnConnect();

	void OnReadPartial(WORD Event, DWORD Total, DWORD Read, char*Buffer);
	void OnReadComplete(WORD Event, DWORD Total, DWORD Read, char*Buffer);

	void OnGetFileInfo();
	void OnContinueDownload();
	void OnEndDownload();
	CMiniDownload(WCHAR* pInit, DWORD Identity);
	~CMiniDownload();
};

