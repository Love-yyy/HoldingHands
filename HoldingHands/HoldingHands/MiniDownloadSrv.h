#pragma once
#include "EventHandler.h"
#include <WinInet.h>
#define MINIDOWNLOAD	('M'|('N'<<8)|('D'<<16)|('D'<<24))

//OnConnect
#define MNDD_STATU_OK						(0x0)
#define MNDD_STATU_ANALYSE_URL_FAILED		(0x1)
#define MNDD_STATU_INTERNETOPEN_FAILED		(0x2)
#define MNDD_STATU_INTERNETCONNECT_FAILED	(0x3)
#define MNDD_STATU_OPENREMOTEFILE_FILED		(0x4)
#define MNDD_STATU_UNKNOWN_FILE_SIZE		(0x5)	//文件大小未知
#define MNDD_STATU_UNSUPPORTED_PROTOCOL		(0x6)
#define MNDD_STATU_HTTPSENDREQ_FAILED		(0x7)

#define MNDD_STATU_OPENLOCALFILE_FAILED		(0x8)

#define MNDD_GET_FILE_INFO		(0xabc0)
#define MNDD_FILE_INFO			(0xabd1)

#define MNDD_DOWNLOAD_CONTINUE	(0xabd3)

//成功,创建文件失败.
#define MNDD_DOWNLOAD_RESULT	(0xabd4)

#define MNDD_DOWNLOAD_END		(0xabd6)

typedef struct MnddFileInfo{
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
typedef struct _DownloadResult
{
	DWORD dwStatu;
	DWORD dwWriteSize;
}DownloadResult;

class CMiniDownloadDlg;

class CMiniDownloadSrv :
	public CEventHandler
{
private:
	CMiniDownloadDlg*m_pDlg;
public:
	void OnClose();					//当socket断开的时候调用这个函数
	void OnConnect();				//当socket连接的时候调用这个函数
	//有数据到达的时候调用这两个函数.
	void OnReadPartial(WORD Event, DWORD Total, DWORD nRead, char*Buffer);
	void OnReadComplete(WORD Event, DWORD Total, DWORD nRead, char*Buffer);
	//有数据发送完毕后调用这两个函数
	void OnWritePartial(WORD Event, DWORD Total, DWORD nWrite, char*Buffer);
	void OnWriteComplete(WORD Event, DWORD Total, DWORD nWrite, char*Buffer);

	void OnGetFileInfoRpl(DWORD dwRead, char*Buffer);
	void OnDownloadRpl(DWORD dwRead, char*Buffer);
	CMiniDownloadSrv(DWORD Identity);
	~CMiniDownloadSrv();
};

