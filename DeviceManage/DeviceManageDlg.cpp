
// DeviceManageDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "DeviceManage.h"
#include "DeviceManageDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDeviceManageDlg �Ի���



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


// CDeviceManageDlg ��Ϣ�������

BOOL CDeviceManageDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	m_camera_dlg = NULL;
	m_camera_dlg_show_status = FALSE;

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CDeviceManageDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CDeviceManageDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CDeviceManageDlg::OnBnClickedButtonDeviceManager()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	if (m_camera_dlg == NULL) {
		CRect rect;
		GetDlgItem(IDC_STATIC_PIC_FOR_DISPLAY_DLG)->GetWindowRect(&rect);
		ScreenToClient(&rect);   //ǧ��ע����ó�ClientToScreen(&rect);  
		m_camera_dlg = new CCameraDlg();
		m_camera_dlg->Create(IDD_DIALOG_CAMERAS, this);
		m_camera_dlg->MoveWindow(rect);
	}
	//other dlg hidden here
	m_camera_dlg->ShowWindow(SW_SHOW);
}
