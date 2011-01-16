// gSoapMFCServerDlg.h : header file
//

#pragma once


// CgSoapMFCServerDlg dialog
class CgSoapMFCServerDlg : public CDialog
{
// Construction
public:
	CgSoapMFCServerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_GSOAPMFCSERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedStart();
    afx_msg void OnTestClicked();
    afx_msg void OnClickedCreateConsole();
    afx_msg void OnClickedFreeConsole();
};
