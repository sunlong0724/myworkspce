#pragma once


// CDisplayDialog �Ի���

class CDisplayDialog : public CDialog
{
	DECLARE_DYNAMIC(CDisplayDialog)

public:
	CDisplayDialog(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDisplayDialog();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_DISPLAY };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
};
