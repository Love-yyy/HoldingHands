#include "IOCPClient.h"
#include "FileMgrSearch.h"


CFileMgrSearch::CFileMgrSearch(DWORD dwIdentity) :
CEventHandler(dwIdentity)
{
}


CFileMgrSearch::~CFileMgrSearch()
{
}


void CFileMgrSearch::OnClose()
{

}

void CFileMgrSearch::OnConnect()
{

}

void CFileMgrSearch::OnReadPartial(WORD Event, DWORD Total, DWORD Read, char*Buffer)
{

}

void CFileMgrSearch::OnReadComplete(WORD Event, DWORD Total, DWORD Read, char*Buffer)
{
	switch (Event)
	{
	case FILE_MGR_SEARCH_SEARCH:
		OnSearch(Buffer);
		break;
	case FILE_MGR_SEARCH_STOP:
		OnStop();
		break;
	default:
		break;
	}
}

struct FindFile
{
	DWORD dwFileAttribute;
	WCHAR szFileName[2];
};

void OnFoundFile(LPCWSTR path, WIN32_FIND_DATA* pfd, LPVOID Param)
{
	//找到一个就发送一个.
	CFileMgrSearch*pMgrSearch = (CFileMgrSearch*)Param;
	DWORD dwLen = sizeof(DWORD) + sizeof(WCHAR)*(wcslen(path) + 1 + wcslen(pfd->cFileName) + 1);
	FindFile*pFindFile = (FindFile*)malloc(dwLen);

	pFindFile->dwFileAttribute = pFindFile->dwFileAttribute;
	wcscpy(pFindFile->szFileName, path);
	wcscat(pFindFile->szFileName, L"\n");
	wcscat(pFindFile->szFileName, pfd->cFileName);

	pMgrSearch->Send(FILE_MGR_SEARCH_FOUND, (char*)pFindFile, dwLen);
	free(pFindFile);
}
void OnSearchOver(LPVOID Param)
{
	CFileMgrSearch*pMgrSearch = (CFileMgrSearch*)Param;
	pMgrSearch->Send(FILE_MGR_SEARCH_OVER, 0, 0);
}

void CFileMgrSearch::OnSearch(char*Buffer)
{
	WCHAR*szStartLocation = (WCHAR*)Buffer;

	WCHAR*szFileName = wcsstr((WCHAR*)Buffer, L"\n");
	if (!szFileName)
		return;
	*szFileName++ = NULL;
	m_searcher.Search(szFileName, szStartLocation, 0, OnFoundFile, OnSearchOver, this);

}
void CFileMgrSearch::OnStop()
{
	m_searcher.StopSearching();
}

/****************************FileTrans Module Entry *********************************/

void BeginSearch(char* szServerAddr,unsigned short uPort,DWORD dwParam)
{
	CIOCPClient Client(szServerAddr,uPort);
	CFileMgrSearch MgrSearch(FILEMGR_SEARCH);

	Client.BindHandler(&MgrSearch);
	Client.Run();
	Client.UnbindHandler();
}
