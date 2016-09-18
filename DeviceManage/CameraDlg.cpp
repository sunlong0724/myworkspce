// CameraDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "DeviceManage.h"
#include "CameraDlg.h"
#include "afxdialogex.h"

#include "Camera.h"

#ifdef _DEBUG
#pragma comment(lib, "DALSACamerad.lib")
#else
#pragma comment(lib, "DALSACamera.lib")
#endif

// CCameraDlg 对话框

IMPLEMENT_DYNAMIC(CCameraDlg, CDialogEx)

CCameraDlg::CCameraDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_CAMERAS, pParent)
{

}

CCameraDlg::~CCameraDlg()
{
}

void CCameraDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_CAMERAS_IP, m_combobox_cameras);
}


BEGIN_MESSAGE_MAP(CCameraDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_SCAN_CAMS, &CCameraDlg::OnBnClickedButtonScanCams)
END_MESSAGE_MAP()


// CCameraDlg 消息处理程序
BOOL CCameraDlg::OnInitDialog() {
	CDialog::OnInitDialog();

	OnBnClickedButtonScanCams();

	return TRUE;
}

void CCameraDlg::OnBnClickedButtonScanCams()
{
	// TODO: 在此添加控件通知处理程序代码
	CCamera::FindCamera(&m_cameras_map);
	m_combobox_cameras.InsertString(0, "Select/Scan");
	auto& a = m_cameras_map.begin();
	for (int i = 0; i < m_cameras_map.size(); ++i) {
		m_combobox_cameras.InsertString(i+1, a->first.c_str());
		++a;
	}
	m_combobox_cameras.SetCurSel(0);
	UpdateData();
}
