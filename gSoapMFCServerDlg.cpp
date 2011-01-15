// gSoapMFCServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "gSoapMFCServer.h"
#include "gSoapMFCServerDlg.h"
#include "soapH.h"
#include "ns.nsmap"
#include "outStackBuffer.h"

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
    ON_BN_CLICKED(IDC_BUTTON1, &CgSoapMFCServerDlg::OnBnClickedStart)
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

HANDLE WINAPI MyThread(LPTHREAD_START_ROUTINE lpStartAddress,
					   LPDWORD lpThreadId,
					   LPVOID lpParameter = NULL,
					   DWORD dwCreationFlags = 0);
HANDLE MyOpenFile(
                LPCTSTR lpszFilename,
                DWORD dwDesiredAccess, 
                DWORD dwShareMode);
void WriteLog(const char* info_format, ...);
HRESULT GenerateConfigTable();
void my_soap_init(struct soap *pSoap);
void SoapErr(struct soap *soap);
LPCTSTR pszCfgFile = _T("./test.ini");
HANDLE hSoapServerThd = NULL;
DWORD soapSvrThdid;
BOOL startSvr = false;

DWORD WINAPI gSoapServer(LPVOID lpThreadParam)
{
	startSvr = true;
	//ServiceService calc_service;
	//calc_service.serve();
	
	// soap_serve(soap_new()); 
	// use the service operation request dispatcher
	// open the log file.

    int i = 1;
    
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
            s = soap_accept(&calc_soap);
            if (s < 0)
            {
                SoapErr(&calc_soap);
                MessageBox(0, _T("soap_accept Error!"), _T("Error"), MB_OK);
                break;
            }
            // fprintf(...
			WriteLog("Thread %d accept socket %d connection from IP %d.%d.%d.%d, request %d", 
				soapSvrThdid, s, (calc_soap.ip >> 24) & 0xFF, 
				(calc_soap.ip >> 16) & 0xFF, 
				(calc_soap.ip >> 8) & 0xFF, calc_soap.ip & 0xFF, i++);
			//soap_clr_mode(&calc_soap, SOAP_C_UTFSTRING);
			//soap_set_mode(&calc_soap, SOAP_C_MBSTRING);
			//soap_omode(&calc_soap, SOAP_C_MBSTRING);

            // process RPC request
			if (soap_serve(&calc_soap) != SOAP_OK)
			{}
			
			WriteLog("request served");
			// clean up class instances
			// deallocate C++ data of old thread
			soap_destroy(&calc_soap); 
			// clean up everything and close socket
			// deallocate data of old thread
			soap_end(&calc_soap); 			
		}
    }
    
    soap_done(&calc_soap);
    MessageBox(0, _T("soap_done!"), _T("Info"), MB_OK);

	startSvr = false;
	WriteLog("Web Server End........");

    return 0;
}

// Implementation of the "add" service operation
int ns__add(struct soap *calc_soap, double a, double b, double &result)
{
    result = a + b;
	WriteLog("%lf + %lf = %lf", a, b, result);
    return SOAP_OK;
}

// Implementation of the "winconfig" service operation
int ns__winconfig(struct soap*, char *key, char *value, bool &result)
{
    result = false;
	WriteLog("%s = %s", key, value);
	if (strlen(key) == 0)
	{
        result = false;
        return -1;
	}
	HANDLE hCfgFile = MyOpenFile(
	                            pszCfgFile, 
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE);
    if (hCfgFile == INVALID_HANDLE_VALUE)
    {
        result = false;
        return HRESULT_FROM_WIN32(GetLastError());
    }
    DWORD FindKey(HANDLE, LPSTR, LPSTR);
    DWORD pFile = FindKey(hCfgFile, key, value);
    CloseHandle(hCfgFile);
    if (pFile == 0)
    {
        result = false;
    }
    else
    {
        result = true;
    }
    
    return SOAP_OK;
}

DWORD GetFileCurrentPointer(HANDLE hFile)
{
    return SetFilePointer(hFile, 0, NULL, FILE_CURRENT);
}

DWORD FindKey(HANDLE hCfgFile, LPSTR key, LPSTR val)
{
    int ConfigChangeState(int*, char*);
    const int BUFFER_SIZE = 128;
    outStackBuffer stack;
    initStack(&stack, (strlen(key) + strlen(val) + BUFFER_SIZE) * 2);
    LPSTR pBuffer = new CHAR[BUFFER_SIZE];
    LPSTR pKey = key;
    DWORD dwBytesRead = 0;
    DWORD dwBytesWritten = 0;
    bool find = false;
    bool end = false;
    int i = 0;
    int rStart = -1;
    int wStart = -1;
    int status = 0;
    //SetFilePointer();
    pushStack(&stack, key, strlen(key));
    pushStack(&stack, "=");
    pushStack(&stack, val, strlen(val));
    pushStack(&stack, "\r");
    pushStack(&stack, "\n");
    do {
        i = 0;
        dwBytesRead = 0;
        dwBytesWritten = 0;
        ZeroMemory(pBuffer, BUFFER_SIZE);
        if (ReadFile(hCfgFile, pBuffer, BUFFER_SIZE - 1, &dwBytesRead, NULL))
        {
            // WriteLog("[read  %dB]%s", dwBytesRead, pBuffer);
            rStart = GetFileCurrentPointer(hCfgFile);
            for (i = 0; i < dwBytesRead; i++)
            {
                ConfigChangeState(&status, pBuffer + i);
                if (find)
                {
                    switch(status)
                    {
						case 0:
                        {
                            if (end)
                            {
                                pushStack(&stack, pBuffer + i);
						    }
						    else
						    {
						        end = true;
						    }
						    break;
                        }
                        case 1:
                        {
                            if (end)
                            {
                                pushStack(&stack, pBuffer + i);
                            }
                            else
                            {
                                if (*(pBuffer + i) != ' ' || *(pBuffer + i) != '\t')
                                {
                                    find = false;
                                    end = false;
                                }
                            }
                            break;
                        }
                        default:
                        {
                            if (end)
                            {
                                pushStack(&stack, pBuffer + i);
                            }
                        }
                    }
                }
                else
                {
                    if (*(pBuffer + i) == *pKey)
                    {
						pKey++;
                        if (*pKey == '\0')
                        {
                            find = true;
                        }
                        if (wStart < 0)
                        {
                            wStart = GetFileCurrentPointer(hCfgFile) - dwBytesRead + i;
                        }    
                    }
                    else
                    {
                        wStart = -1;
                        rStart = -1;
                        pKey = key;
                    }
                }
            }
            if (find && end && ((wStart + stack.index < rStart) || (dwBytesRead < BUFFER_SIZE - 1)))
            {
                SetFilePointer(hCfgFile, wStart, NULL, FILE_BEGIN);
                if (WriteFile(hCfgFile, stack.pBuffer, stack.index, &dwBytesWritten, NULL))
                {
                    wStart += dwBytesWritten;
                    emptyStack(&stack);
                }
                SetFilePointer(hCfgFile, rStart, NULL, FILE_BEGIN);
            }
        }
    } while (!(dwBytesRead < BUFFER_SIZE - 1));
    if (stack.index > 0)
    {
        if (WriteFile(hCfgFile, stack.pBuffer, stack.index, &dwBytesWritten, NULL))
        {
            wStart += dwBytesWritten;
            emptyStack(&stack);
        }
    }
    delete[] pBuffer;
    deleteStack(&stack);
    return dwBytesWritten;
}


void CgSoapMFCServerDlg::OnBnClickedStart()
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

void WriteLog(const char* info_format, ...)
{
    SYSTEMTIME timeNow;
    GetLocalTime(&timeNow);
	va_list arg_ptr;
	va_start(arg_ptr, info_format);
    FILE *pfLog;
    //time_t t;
    //time(&t);
    char time_str[64] = {0};
    sprintf(time_str, "%4d/%2d/%2d %2d:%2d:%2d %4dms ... ", timeNow.wYear, timeNow.wMonth, timeNow.wDay,
        timeNow.wHour, timeNow.wMinute, timeNow.wSecond, timeNow.wMilliseconds);
    //strftime(time_str, sizeof(time_str), "%Y/%m/%d %H:%M:%S %z...", localtime(&t));
	pfLog = fopen("soap.log", "a+");
	fprintf(pfLog, "%s", time_str);
	vfprintf(pfLog, info_format, arg_ptr);
	fprintf(pfLog, "\n");
    fclose(pfLog);
	va_end(arg_ptr);
}

void SoapErr(struct soap *soap)
{
    WriteLog(NULL);
    soap_print_fault(soap, stderr);
}

HANDLE MyOpenFile(
                LPCTSTR lpszFilename,
                DWORD dwDesiredAccess, 
                DWORD dwShareMode)
{
    return CreateFile(
                    lpszFilename, 
                    dwDesiredAccess,
                    dwShareMode,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
}

HANDLE OpenWebFile(LPCTSTR lpszFilename)
{
    return MyOpenFile(lpszFilename, 
                    GENERIC_READ,
                    FILE_SHARE_READ);
}

HRESULT ReadFileToBuffer(HANDLE hFile, LPVOID read_buf, DWORD buf_len, LPDWORD lpdwBytesRead)
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

int ConfigChangeState(int *pState, char *pCh)
{
    switch(*pState)
    {
        case 0:
        {
            if (*pCh == '=')
            {
                *pState = 2;
            }
            else if (*pCh == '\r' || *pCh == '\n')
            {
                // *pState = 4;
            }
            else
            {
                *pState = 1;
            }
            break;
        }
        case 1:
        {
            if (*pCh == '=')
            {
                *pState = 2;
            }
            else if (*pCh == '\r' || *pCh == '\n')
            {
                *pState = 4;
            }
            break;
        }
        case 2:
        {
            if (*pCh == '\r' || *pCh == '\n')
            {
                *pState = 4;
            }
            else if (*pCh != '=')
            {
                *pState = 3;
            }
            break;
        }
        case 3:
        {
            if (*pCh == '\r' || *pCh == '\n')
            {
                *pState = 4;
            }
            break;
        }
        case 4:
        {
            if (*pCh == '\r' || *pCh == '\n')
            {
                *pState = 0;
            }
            else if (*pState == '=')
            {
                *pState = 2;
            }
			else
			{
                *pState = 1;
			}
            break;
        }
        default: {}
    }
    return *pState;
}

int PushStackAsStateChanged(poutStackBuffer pStack, int *pState, char *pCh)
{
    int preState = *pState;
    ConfigChangeState(pState, pCh);
    switch (preState)
    {
        case 0:
        {
            switch (*pState)
            {
                case 1:
                {
    				pushStack(pStack, "<tr><td>", 8);
                    break;
                }
                case 2:
                {
    				pushStack(pStack, "<tr><td></td>", 13);
                    break;
                }
                default: {}
            }
            break;
        }
        case 1:
        {
            switch (*pState)
            {
                case 2:
                {
    				pushStack(pStack, "</td>", 5);
                    break;
                }
                case 4:
                {
    				pushStack(pStack, "</td><td>=</td><td></td></tr>", 29);
                    break;
                }
                default: {}
            }
            break;
        }
        case 2:
        {
            switch (*pState)
            {
                case 3:
                {
				    LPCSTR html= "<td>=</td><td>";
				    pushStack(pStack, html, strlen(html));
                    break;
                }
                case 4:
                {
    				pushStack(pStack, "<td>=</td><td></td></tr>", 24);
                    break;
                }
                default: {}
            }
            break;
        }
        case 3:
        {
            switch (*pState)
            {
                case 4:
                {
				    LPCSTR html = "</td></tr>";
				    pushStack(pStack, html, strlen(html));
                    break;
                }
                default: {}
            }
            break;
        }
        case 4:
        {
            switch (*pState)
            {
                case 1:
                {
                    pushStack(pStack, "<tr><td>", 8);
                    break;
                }
                case 2:
                {
                    pushStack(pStack, "<tr><td></td>", 13);
                    break;
                }
                default: {}
            }
            break;
        }
        default: {}
    }
    return *pState;
}

HRESULT SendConfigTable(struct soap* pSoap, poutStackBuffer pStack)
{
    HRESULT hr;
    const int BUFFER_SIZE = 128;
    HANDLE hCfgFile = OpenWebFile(pszCfgFile);
    if (hCfgFile == INVALID_HANDLE_VALUE) 
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
    }
    LPSTR lpBuffer = new CHAR[BUFFER_SIZE];
    if (lpBuffer)
    {
        ZeroMemory(lpBuffer, BUFFER_SIZE);
    }

    LPCSTR table = "<table id='config'>";
    pushStack(pStack, table, strlen(table));
    DWORD dwBytesRead = 0;
    int state = 0;
    do {
        hr = ReadFileToBuffer(hCfgFile, 
            lpBuffer, BUFFER_SIZE, &dwBytesRead);
        //CopyMemory(lpHtmlBuffer, lpBuffer, dwBytesRead);
        int start = 0;
        int indexHtml = 0;
        for (int i = 0; i < dwBytesRead; i++)
        {
			switch (PushStackAsStateChanged(pStack, &state, lpBuffer + i))
			{
				case 1:
				case 3:
				{
					pushStack(pStack, lpBuffer + i);
				}
				default:
					break;
			}
        }
		if (pStack->index > 0)
        {
			soap_send_raw(pSoap, pStack->pBuffer, pStack->index);
			emptyStack(pStack);
        }
    } while (!(dwBytesRead < BUFFER_SIZE));
    pushStack(pStack, "</table>", strlen("</table>"));
    
    if (lpBuffer) 
    {
        delete[] lpBuffer;
        lpBuffer = NULL;
    }
    CloseHandle(hCfgFile);
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

int ChangeState(int *pState, char *pCh)
{
    switch(*pState)
    {
        case 0:
        {
            if (*pCh == '<')
            {
                *pState = 1;
            }
            break;
        }
        case 1:
        {
            if (*pCh == '%')
            {
                *pState = 2;
            }
            else if (*pCh == '<')
            {
            }
            else
            {
                *pState = 0;
            }
            break;
        }
        case 2:
        {
            if (*pCh == '%')
            {
                *pState = 3;
            }
            break;
        }
        case 3:
        {
            if (*pCh == '>')
            {
                *pState = 4;
            }
            else
            {
                *pState = 2;
            }
            break;
        }
        default:
            return *pState;
    }
    
    return *pState;
}

HANDLE SelectFile(struct soap *soap, FileType ft)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	switch (ft)
    {
        // HTTP response header with text/html
        case HTML:
		{
            if (strstr(soap->path, "test"))
            {
                hFile = OpenWebFile(_T("./test.htm"));
            }
            else
            {
                hFile = OpenWebFile(_T("./index.htm"));
            }
            break;
		}
		case JS:
		{
		    if (strstr(soap->path, "main"))
		    {
                hFile = OpenWebFile(_T("./main.js"));
		    }
		    else
		    {
                hFile = OpenWebFile(_T("./jquery-1.4.4.min.js"));
            }
            break;
		}
        case CSS: 
		{
			hFile = OpenWebFile(_T("./main.css"));
            break;
		}
        default:
		{
			if (rand() % 2 + 1)
			{
				hFile = OpenWebFile(_T("./ns.winconfig.req.xml"));
			}
			else
			{
				hFile = OpenWebFile(_T("./ns.add.req.xml"));
			}
		}
    }

	return hFile;
}

int MyHttpGet(struct soap *soap)
{
    const int BUFFER_SIZE = 1024 * 8;
    HRESULT hr = S_OK;
    HANDLE hFile;
    DWORD dwBytesRead = 0;
    char read_buf[BUFFER_SIZE] = {0};
    outStackBuffer bufStack;
	initStack(&bufStack, BUFFER_SIZE * 2);
    
	soap_response(soap, SOAP_FILE);
	soap->http_content = "text/html; charset=gb2312";
	FileType ft = GetFileType(soap->path);
	hFile = SelectFile(soap, ft);

    if (hFile == INVALID_HANDLE_VALUE) 
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
    }
    if (SUCCEEDED(hr))
    {
        int status = 0;
		int pushed = -1;
        do {
            int start = -1;
            int end = -1;
            ReadFileToBuffer(hFile,
                read_buf, 
                BUFFER_SIZE, 
                &dwBytesRead);
            if (ft == HTML)
            {
                // search for <% %>
                for (int i = 0; i < dwBytesRead; i++)
                {
                    switch (ChangeState(&status, read_buf + i))
                    {
						case 0:
						{
							while (pushed < i)
							{
								pushed++;
								if (pushed < 0)
								{
									pushStack(&bufStack, "<");
								}
								else
								{
									pushStack(&bufStack, read_buf + pushed);
								}
							}
							break;
						}
                        case 1:
                        {
							if (pushed + 1 < i)
							{
								pushed++;
								if (pushed < 0)
								{
									pushStack(&bufStack, "<");
								}
								else
								{
									pushStack(&bufStack, read_buf + pushed);
								}
							}
                            break;
                        }
                        case 4:
                        {
                            pushed = i;
							status = 0;
							SendConfigTable(soap, &bufStack);
							break;
                        }
						default:
							break;
                    }
                }
				if (status == 1)
				{
					pushed -= dwBytesRead;
				}
				else
				{
					pushed = -1;
				}
                //if (status == 0)
                //{
                //    soap_send_raw(soap, read_buf, dwBytesRead);
                //}
                //if (status > 0 && !(start < 0))
                //{
                //    soap_send_raw(soap, read_buf, start);
                //}
                //if (status > 3 && end > 0 && (dwBytesRead - end) > 0)
                //{
                //    soap_send_raw(soap, read_buf + end, dwBytesRead - end);
                //}
                //if (status > 3 && start < 0 && end < 0)
                //{
                //    soap_send_raw(soap, read_buf, dwBytesRead);
                //}
				if (bufStack.index > 0)
				{
					soap_send_raw(soap, bufStack.pBuffer, bufStack.index);
					emptyStack(&bufStack);
				}
            }
            else
            {
                soap_send_raw(soap, read_buf, dwBytesRead);
            }
        } while (!(dwBytesRead < BUFFER_SIZE));
    }    
    soap_end_send(soap);
    CloseHandle(hFile);
	WriteLog("MyHttpGet %s", soap->path);
	deleteStack(&bufStack);

    return SOAP_OK;
}

HANDLE WINAPI MyThread(
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPDWORD lpThreadId,
    LPVOID lpParameter,
    DWORD dwCreationFlags)
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

#define BACKLOG (100) // Max. request backlog
#define MAX_THR (3) // Max. threads to serve requests
#define MAX_QUEUE (100) // Max. size of request queue

SOAP_SOCKET queue[MAX_QUEUE]; // The global request of sockets
int head = 0, tail = 0; // Queue head and tail
//pthread_mutex_t queue_cs;
//pthread_cond_t queue_cv;


void my_soap_init(struct soap *pSoap)
{
	pSoap->send_timeout = 60; // 60 seconds
	pSoap->recv_timeout = 60;
	//pSoap->tc
	//pSoap->accept_timeout = 0;  无限等待连接请求
	//pSoap->max_keep_alive = 100;
	//soap_clr_mode(pSoap, SOAP_C_UTFSTRING);
	//soap_set_mode(pSoap, SOAP_C_MBSTRING);
	//soap_omode(&calc_soap, SOAP_C_MBSTRING);
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
					tid[i], s, (calc_soap.ip >> 24) & 0xFF, 
					(calc_soap.ip >> 16) & 0xFF, 
					(calc_soap.ip >> 8) & 0xFF, calc_soap.ip & 0xFF);
                if (!ptsoap[i]) // first time around
                {
                    ptsoap[i] = soap_copy(&calc_soap);
					if (!ptsoap[i])
					{
						ASSERT(0);
						exit(1);
						// error
					}
                }
				else
				{
					WaitForSingleObject(th[i], 1 * 1000);
					WriteLog("Thread %d[%d] completed, status tid = %d", th[i], i, tid[i]);
					// deallocate C++ data of old thread
					soap_destroy(ptsoap[i]); 
					// deallocate data of old thread
					soap_end(ptsoap[i]); 
				}
				// new socket fd
				ptsoap[i]->socket = s;
				th[i] = MyThread(ProcessRequest, &tid[i], ptsoap[i]);
            }

			WaitForMultipleObjects(MAX_THR, th, TRUE, 1 * 1000);
            for (i = 0; i < MAX_THR; i++)
            {
				CloseHandle(th[i]);
				if (ptsoap[i])
				{
					soap_done(ptsoap[i]); //detach context
					free(ptsoap[i]); //free up
				}
            }

			startSvr = false;
        }
    }
    
    soap_done(&calc_soap);
    MessageBox(0, _T("soap_done!"), _T("Info"), MB_OK);

	startSvr = false;
	WriteLog("Web Server End........");

    return 0;
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