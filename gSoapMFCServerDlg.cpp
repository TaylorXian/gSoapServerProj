// gSoapMFCServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "gSoapMFCServer.h"
#include "gSoapMFCServerDlg.h"
#include "soapH.h"
#include "ns.nsmap"

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
    ON_BN_CLICKED(IDC_BUTTON1, &CgSoapMFCServerDlg::OnBnClickedButton1)
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

// Implementation of the "add" service operation
int ns__add(struct soap *calc_soap, double a, double b, double &result)
{
    result = a + b;
    return SOAP_OK;
}

void WriteLog(const char* info)
{
    FILE *pfLog;
    time_t t;
    time(&t);
    char time_str[64] = {0};
    strftime(time_str, sizeof(time_str), "%Y/%m/%d %H:%M:%S %z...", localtime(&t));
	pfLog = fopen("soap.log", "a+");
	fprintf(pfLog, "%s%s\n", time_str, info);
    fclose(pfLog);
}

HANDLE OpenWebFile(LPCTSTR lpszFilename)
{
    return CreateFile(lpszFilename, 
    GENERIC_READ,
    FILE_SHARE_READ,
    NULL,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL,
    NULL);
}

HRESULT ReadFileToBuffer(HANDLE hFile, LPSTR read_buf, DWORD buf_len, LPDWORD lpdwBytesRead)
{
    HRESULT hr = S_OK;
    ZeroMemory(read_buf, buf_len);
    *lpdwBytesRead = 0;
    if (FALSE == ReadFile(hFile, read_buf, buf_len, lpdwBytesRead, NULL))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    
    return hr;
}

typedef enum {
    HTML, JS, CSS, OTHER
} FileType;

FileType get_filetype(const char* path)
{
    if (path)
    {
        if (strstr(path, ".htm") || !strcmp(path, "/"))
        {
            return HTML;
        }
        if (strstr(path, ".js")) 
        {
            return JS;
        }
        if (strstr(path, ".css"))
        {
            return CSS;
        }
    }
    return OTHER;
}

int my_http_get(struct soap *soap)
{
    const int BUFFER_SIZE = 1024 * 4;
    HRESULT hr = S_OK;
    HANDLE hFile;
    DWORD dwBytesRead = 0;
    char read_buf[BUFFER_SIZE] = {0};
    
    switch (get_filetype(soap->path))
    {
        // HTTP response header with text/html
        case HTML: soap_response(soap, SOAP_HTML);
            hFile = OpenWebFile(_T("./index.htm"));
            break;
        case JS: soap_response(soap, SOAP_FILE);
            hFile = OpenWebFile(_T("./jquery-1.4.4.min.js"));
            break;
        case CSS: soap_response(soap, SOAP_FILE);
            break;
        default: soap_response(soap, SOAP_FILE);
            hFile = OpenWebFile(_T("./ns.add.req.xml"));
    }
    if (hFile == INVALID_HANDLE_VALUE) 
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
    }
    if (SUCCEEDED(hr))
    {
        do {
            ReadFileToBuffer(hFile,
                read_buf, 
                BUFFER_SIZE, 
                &dwBytesRead);
            soap_send(soap, read_buf);
        } while (!(dwBytesRead < BUFFER_SIZE));
    }    
    soap_end_send(soap);
    CloseHandle(hFile);

    return SOAP_OK;
}

HANDLE hSoapServerThd = NULL;
DWORD soap_svr_thdid;
BOOL start_svr = false;

DWORD WINAPI StartgSoapServer(LPVOID lpThreadParameter)
{
	//ServiceService calc_service;
	//calc_service.serve();
	
	// soap_serve(soap_new()); 
	// use the service operation request dispatcher
	// open the log file.

	struct soap calc_soap;
	int m, s; // master and slave sockets
	soap_init(&calc_soap);
	calc_soap.fget = my_http_get;
	m = soap_bind(&calc_soap, 
	    NULL, // 任何IP地址都可以访问
	    18083, // 端口
	    100); // 请求队列的长度
	if (m < 0)
	{
	    WriteLog("Start Server Error!");
	    // MessageBox(0, _T("Start Server Error!\n"), _T("Info"), MB_OK);
	}
	else
	{
	    WriteLog("Start Server successful........");
        MessageBox(0, _T("Start Server successful........"), _T("Info"), MB_OK);
        while (start_svr)
        {
        s = soap_accept(&calc_soap);
        if (s < 0)
        {
            MessageBox(0, _T("soap_accept Error!"), _T("Error"), MB_OK);
        }
        if (soap_serve(&calc_soap) != SOAP_OK) //
        {
            MessageBox(0, _T("soap_serve Error!"), _T("Error"), MB_OK);
        }
        soap_destroy(&calc_soap);
        soap_end(&calc_soap);
        }
    }
    
    soap_done(&calc_soap);
    MessageBox(0, _T("soap_done!"), _T("Info"), MB_OK);
    
    return 0;
}

void CgSoapMFCServerDlg::OnBnClickedButton1()
{
    // TODO: Add your control notification handler code here
    if (!start_svr)
    {
        start_svr = true;
        hSoapServerThd = CreateThread(NULL, 0, StartgSoapServer, NULL, 0, &soap_svr_thdid);
    }
    else
    {
        
    }
    // StartgSoapServer(NULL);
}
