
// DeviceManageDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "CameraDlg.h"

// CDeviceManageDlg �Ի���
class CDeviceManageDlg : public CDialog
{
// ����
public:
	CDeviceManageDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DEVICEMANAGE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

	CCameraDlg*		m_camera_dlg;
	BOOL			m_camera_dlg_show_status;

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonDeviceManager();
	// //main display
};
