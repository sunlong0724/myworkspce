
// DeviceManageDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "DeviceManage.h"
#include "DeviceManageDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDeviceManageDlg 对话框



CDeviceManageDlg::CDeviceManageDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DEVICEMANAGE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDeviceManageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDeviceManageDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_DEVICE_MANAGER, &CDeviceManageDlg::OnBnClickedButtonDeviceManager)
END_MESSAGE_MAP()


// CDeviceManageDlg 消息处理程序

BOOL CDeviceManageDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_camera_dlg = NULL;
	m_camera_dlg_show_status = FALSE;

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CDeviceManageDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CDeviceManageDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CDeviceManageDlg::OnBnClickedButtonDeviceManager()
{
	// TODO: 在此添加控件通知处理程序代码

	if (m_camera_dlg == NULL) {
		CRect rect;
		GetDlgItem(IDC_STATIC_PIC_FOR_DISPLAY_DLG)->GetWindowRect(&rect);
		ScreenToClient(&rect);   //千万注意别用成ClientToScreen(&rect);  
		m_camera_dlg = new CCameraDlg();
		m_camera_dlg->Create(IDD_DIALOG_CAMERAS, this);
		m_camera_dlg->MoveWindow(rect);
	}
	//other dlg hidden here
	m_camera_dlg->ShowWindow(SW_SHOW);
}
