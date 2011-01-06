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

void WriteLog(const char* info_format, ...)
{
	va_list arg_ptr;
	va_start(arg_ptr, info_format);
    FILE *pfLog;
    time_t t;
    time(&t);
    char time_str[64] = {0};
    strftime(time_str, sizeof(time_str), "%Y/%m/%d %H:%M:%S %z...", localtime(&t));
	pfLog = fopen("soap.log", "a+");
	fprintf(pfLog, "%s", time_str);
	fprintf(pfLog, info_format, arg_ptr);
	fprintf(pfLog, "\n");
    fclose(pfLog);
	va_end(arg_ptr);
}

void SoapErr(struct soap *soap)
{
    WriteLog(NULL);
    soap_print_fault(soap, stderr);
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

FileType GetFileType(const char* path)
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

int MyHttpGet(struct soap *soap)
{
    const int BUFFER_SIZE = 1024 * 4;
    HRESULT hr = S_OK;
    HANDLE hFile;
    DWORD dwBytesRead = 0;
    char read_buf[BUFFER_SIZE] = {0};
    
    switch (GetFileType(soap->path))
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

HANDLE WINAPI MyThread(
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPDWORD lpThreadId,
    LPVOID lpParameter = NULL,
    DWORD dwCreationFlags = 0)
{
    return CreateThread(NULL, 0, 
        lpStartAddress, 
        lpParameter, 
        dwCreationFlags, 
        lpThreadId);
}

DWORD WINAPI ProcessRequest(LPVOID lpThreadParam)
{
    struct soap *psoap = (struct soap *)lpThreadParam;
    if (soap_serve(psoap) != SOAP_OK) //
    {
        MessageBox(0, _T("soap_serve Error!"), _T("Error"), MB_OK);
    }
    soap_destroy(psoap);
    soap_end(psoap);
    soap_done(psoap);
    free(psoap);
    return 0;
}

HANDLE hSoapServerThd = NULL;
DWORD soapSvrThdid;
BOOL startSvr = false;
#define BACKLOG (100) // Max. request backlog
#define MAX_THR (5) // Max. threads to serve requests
#define MAX_QUEUE (100) // Max. size of request queue

SOAP_SOCKET queue[MAX_QUEUE]; // The global request of sockets
int head = 0, tail = 0; // Queue head and tail
//pthread_mutex_t queue_cs;
//pthread_cond_t queue_cv;


void my_soap_init(struct soap *pSoap)
{
	pSoap->send_timeout = 60; // 60 seconds
	pSoap->recv_timeout = 60;
	pSoap->accept_timeout = 60;
	//pSoap->max_keep_alive = 100;
	pSoap->fget = MyHttpGet;
}

DWORD WINAPI StartgSoapServer(LPVOID lpThreadParam)
{
	startSvr = true;
	//ServiceService calc_service;
	//calc_service.serve();
	
	// soap_serve(soap_new()); 
	// use the service operation request dispatcher
	// open the log file.

    struct soap* ptsoap[MAX_THR] = {0};
    HANDLE th[MAX_THR] = {0};
    DWORD tid[MAX_THR] = {0};
    int i;
    
	struct soap calc_soap;
	int m, s; // master and slave sockets
	// soap init
	soap_init(&calc_soap);
	my_soap_init(&calc_soap);
	
	// 
	m = soap_bind(&calc_soap, 
	    NULL, // 任何IP地址都可以访问
	    18083, // 端口
	    100); // 请求队列的长度
	if (m < 0) //!soap_valid_socket(m)
	{
	    WriteLog("Start Server Error!");
	    // 
	    MessageBox(0, _T("Start Server Error!\n"), _T("Info"), MB_OK);
	}
	else
	{
	    WriteLog("Start Server successful........");
        MessageBox(0, _T("Start Server successful........"), _T("Info"), MB_OK);
        while (startSvr)
        {
            for (i = 0; i < MAX_THR; i++)
            {
                s = soap_accept(&calc_soap);
                if (s < 0)
                {
                    SoapErr(&calc_soap);
                    MessageBox(0, _T("soap_accept Error!"), _T("Error"), MB_OK);
                    break;
                }
                // fprintf(...
				WriteLog("Thread %d accept socket %d connection from IP %d.%d.%d.%d", 
					i, s, (calc_soap.ip >> 24) & 0xFF, 
					(calc_soap.ip >> 16) & 0xFF, 
					(calc_soap.ip >> 8) & 0xFF, calc_soap.ip & 0xFF);
                if (!ptsoap[i]) // first time around
                {
                    ptsoap[i] = soap_copy(&calc_soap);
                }
                if (!ptsoap[i])
                {
					ASSERT(0);
					exit(1);
                    // error
                }
				WaitForSingleObject(th[i], 1 * 1000);
				WriteLog("Thread %d[%d] completed, status tid = %d\n", th[i], i, tid[i]);
				// deallocate C++ data of old thread
				soap_destroy(ptsoap[i]); 
				// deallocate data of old thread
				soap_end(ptsoap[i]); 
				// new socket fd
				ptsoap[i]->socket = s;
				th[i] = MyThread(ProcessRequest, &tid[i], ptsoap[i]);
            }
            for (i = 0; i < MAX_THR; i++)
            {
            }
        }
    }
    
    soap_done(&calc_soap);
    MessageBox(0, _T("soap_done!"), _T("Info"), MB_OK);

	startSvr = false;
	WriteLog("Web Server End........");

    return 0;
}

void CgSoapMFCServerDlg::OnBnClickedButton1()
{
    // TODO: Add your control notification handler code here
    if (!startSvr)
    {
        hSoapServerThd = MyThread(StartgSoapServer, &soapSvrThdid);
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
    // StartgSoapServer(NULL);
}

//void* process_queue(void* soap);
//int enqueue(SOAP_SOCKET sock);
//SOAP_SOCKET dequeue();
//void* process_queue(void* soap)
//{
//    struct soap* tsoap = (struct soap*)soap;
//    for (;;)
//    {
//        tsoap->socket = dequeue();
//        if (0)
//        {
//            break;
//        }
//        soap_serve(tsoap);
//        soap_destroy(tsoap);
//        soap_end(tsoap);
//        fprintf(stderr, "served\n");
//    }
//    
//    return NULL;
//}
//
//int enqueue(SOAP_SOCKET sock)
//{
//    int status = SOAP_OK;
//    int next;
//    pthread_mutex_lock(&queue_cs);
//    next = tail + 1;
//    if (next >= MAX_QUEUE)
//    {
//        next = 0;
//    }
//    if (next == head)
//    {
//        status = SOAP_EOM;
//    } else {
//        queue[tail] = sock;
//        tail = next;
//    }
//    pthread_cond_signal(&queue_cv);
//    pthread_mutex_unlock(&queue_cs);
//    
//    return status;
//}
//
//SOAP_SOCKET dequeue()
//{
//    SOAP_SOCKET sock;
//    pthread_mutex_lock(&queue_cs);
//    while (head == tail)
//    {
//        pthread_cond_wait(&queue_cv, &queue_cs);
//    }
//    sock = queue[head++];
//    if (head >= MAX_QUEUE)
//    {
//        head = 0;
//    }
//    pthread_mutex_unlock(&queue_cs);
//    return sock;
//}