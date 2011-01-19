// gSoapMFCServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "gSoapMFCServer.h"
#include "gSoapMFCServerDlg.h"
#include "MygSoapServer.h"

// 部署到Wince上时 这里要修改
LPCSTR lpszFormat = ".%s";


#ifdef _DEBUG
#define new DEBUG_NEW
#endif



// CgSoapMFCServerDlg dialog




CgSoapMFCServerDlg::CgSoapMFCServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CgSoapMFCServerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CgSoapMFCServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CgSoapMFCServerDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_BTN_START_SERVER, &CgSoapMFCServerDlg::OnClickedStartSvr)
    ON_BN_CLICKED(IDC_BTN_RUN_TESTS, &CgSoapMFCServerDlg::OnClickedTest)
    ON_BN_CLICKED(IDC_BTN_ALLOC_CONSOLE, &CgSoapMFCServerDlg::OnClickedCreateConsole)
    ON_BN_CLICKED(IDC_BTN_FREE_CONSOLE, &CgSoapMFCServerDlg::OnClickedFreeConsole)
	ON_BN_CLICKED(IDC_BTN_STOP_SVR, &CgSoapMFCServerDlg::OnClickedStopSvr)
END_MESSAGE_MAP()


// CgSoapMFCServerDlg message handlers

BOOL CgSoapMFCServerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CgSoapMFCServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CgSoapMFCServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CgSoapMFCServerDlg::OnClickedStartSvr()
{
    // TODO: Add your control notification handler code here
    
    if (!startSvr)
    {
        hSoapServerThd = MyThread(gSoapServer, &soapSvrThdid);
    }
    else
    {
		startSvr = false;
		::MessageBox(0, 
			_T("WebServer have been running!"), 
			_T("Info"), MB_OK);
		::MessageBox(0, 
			_T("WebServer will be stoped!\nMaybe need another request!"), 
			_T("Info"), MB_OK);
    }

    //struct soap s;
    //bool b;
    //HRESULT hr = ns__winconfig(&s, "TFTPServerIP", "", b);
    //HRESULT hr = SendConfigTable();

    // StartgSoapServer(NULL);
}

BOOL promptCreated = false;
void CgSoapMFCServerDlg::OnClickedTest()
{
    // TODO: Add your control notification handler code here
    if (promptCreated)
    {
        AllTests();
    }
    else
    {
        this->OnClickedCreateConsole();
        this->OnClickedTest();
    }
}

void CgSoapMFCServerDlg::OnClickedCreateConsole()
{
    // TODO: Add your control notification handler code here
    if (!promptCreated)
    {
        promptCreated = AllocConsole();
        if (promptCreated)
        {
            OpenStdConsoleCRT();
        }
    }
}

void CgSoapMFCServerDlg::OnClickedFreeConsole()
{
    // TODO: Add your control notification handler code here
    CloseStdConsoleCRT();
    FreeConsole();
    promptCreated = false;
}

void CgSoapMFCServerDlg::OnClickedStopSvr()
{
	// TODO: Add your control notification handler code here
	if (hSoapServerThd != NULL)
	{
		CloseHandle(hSoapServerThd);
		hSoapServerThd = NULL;
	}
}
