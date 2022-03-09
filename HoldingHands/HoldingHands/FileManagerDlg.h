#pragma once
#include "afxcmn.h"
#include "resource.h"


#define WM_FMDLG_CHDIR	(WM_USER+133)
#define WM_FMDLG_LIST	(WM_USER+134)

#define WM_FMDLG_TEST	(WM_USER+135)

#define WM_FMDLG_PREV_UPLOAD_FROMDISK	(WM_USER+136)
#define WM_FMDLG_PREV_UPLOAD_FROMURL	(WM_USER+137)
#define WM_FMDLG_PREV_DOWNLOAD			(WM_USER+138)
#define WM_FMDLG_PREV_RENAME			(WM_USER+139)
#define WM_FMDLG_PREV_NEWFOLDER			(WM_USER+140)

// CFileManagerDlg 对话框	
class CFileManagerSrv;

typedef struct DriverInfo
{
	WCHAR szName[128];
	WCHAR szTypeName[128];
	WCHAR szFileSystem[128];
	ULARGE_INTEGER	Total;
	ULARGE_INTEGER	Free;
	DWORD dwType;
}DriverInfo;


typedef struct FMFileInfo
{
	DWORD dwFileAttribute;
	DWORD dwFileSizeLo;
	DWORD dwFileSizeHi;
	DWORD dwLastWriteLo;
	DWORD dwLastWriteHi;
	WCHAR szFileName[2];
}FMFileInfo;

class CFileManagerDlg : public CDialogEx
{
	
	DECLARE_DYNAMIC(CFileManagerDlg)

public:
	CFileManagerDlg(CFileManagerSrv*pHandler, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CFileManagerDlg();

// 对话框数据
	enum { IDD = IDD_FM_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	static HIMAGELIST hImageList_SmallIcon;
	static HIMAGELIST hImageList_LargeIcon;
	static CImageList	*pLargeImageList;		//File List Icon.
	static CImageList	*pSmallImageList;

	CFileManagerSrv*	m_pHandler;
	
	virtual BOOL OnInitDialog();
	CListCtrl m_FileList;
	CString m_Location;
	CToolBar	m_ToolBar;
	CImageList	m_ImageList;			//tool bar image list
	
	void Resize();
	void FillDriverList(DriverInfo*pDis,int count);

	void FillFileList(FMFileInfo*pfis);

	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();

	afx_msg void OnToolbarDropDown(NMHDR* pnmhdr, LRESULT* plRes);
	afx_msg void OnViewIcon();
	afx_msg void OnViewDetailedinformation();
	afx_msg void OnViewList();



	afx_msg LRESULT OnChDir(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnFillList(WPARAM wParam, LPARAM lParam);


	afx_msg void OnUp();
	afx_msg void OnGo();
	afx_msg void OnRefresh();
	afx_msg void OnSearch();

	afx_msg void OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnUploadFromdisk();
	afx_msg void OnUploadFromurl();
	afx_msg void OnMenuDownload();

	afx_msg LRESULT OnPrevUploadFromUrl(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPrevUploadFromDisk(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPrevDownload(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnPrevNewfolder(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnPrevRename(WPARAM wParam, LPARAM lParam);

	afx_msg void OnRunfileHide();
	afx_msg void OnRunfileNormal();
	afx_msg void OnMenuRename();
	afx_msg void OnMenuNewfolder();
	virtual void OnOK();
	afx_msg void OnMenuDelete();
	afx_msg void OnMenuCopy();
	afx_msg void OnMenuCut();
	afx_msg void OnMenuPaste();
};
