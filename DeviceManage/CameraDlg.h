#pragma once

#include <map>
#include "afxwin.h"

// CCameraDlg �Ի���

class CCameraDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CCameraDlg)

public:
	CCameraDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CCameraDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CAMERAS };
#endif

	std::map<std::string, std::map<int32_t, std::string>> m_cameras_map;
protected:

	virtual BOOL OnInitDialog();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonScanCams();
	CComboBox m_combobox_cameras;
};
