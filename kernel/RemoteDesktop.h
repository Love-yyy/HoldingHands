#pragma once
#include "EventHandler.h"
#include "DesktopGrab.h"
#define REMOTEDESKTOP	('R'|('D'<<8)|('T'<<16)|('P'<<24))

//Êó±ê?,Í¸Ã÷´°¿Ú?
#define REMOTEDESKTOP_GETSIZE		(0xaa01)
#define REMOTEDESKTOP_DESKSIZE		(0xaa02)

#define REMOTEDESKTOP_NEXT_FRAME	(0xaaa1)
#define REMOTEDESKTOP_FRAME			(0xaaa2)
#define REMOTEDESKTOP_ERROR			(0xaaa3)

#define REMOTEDESKTOP_CTRL			(0xaaa4)

#define REMOTEDESKTOP_SETMAXFPS		(0xaaa5)


class CRemoteDesktop :
	public CEventHandler
{
private:
	typedef struct
	{
		DWORD dwType;
		union
		{
			DWORD dwCoor;
			DWORD VkCode;
		}Param;
		DWORD dwExtraData;
	}CtrlParam;
private:
	CDesktopGrab m_grab;
	DWORD		 m_dwLastTime;
	char*		 m_FrameBuffer;
	DWORD		 m_dwFrameSize;
	DWORD		 m_dwMaxFps;


public:
	void OnClose();
	void OnConnect();

	void OnReadPartial(WORD Event, DWORD Total, DWORD Read, char*Buffer);
	void OnReadComplete(WORD Event, DWORD Total, DWORD Read, char*Buffer);

	void OnGetSize();

	void OnNextFrame(char*flag);
	void OnControl(CtrlParam*Param);
	void OnSetMaxFps(DWORD dwMaxFps);

	CRemoteDesktop(DWORD dwIdentity);
	~CRemoteDesktop();
};

