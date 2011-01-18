// gSoapMFCServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "gSoapMFCServer.h"
#include "gSoapMFCServerDlg.h"
#include "soapH.h"
#include "ns.nsmap"
#include "outStackBuffer.h"
#include "MyUtil.h"
#include "MyFileParser.h"
#include "alltests.h"

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

HANDLE WINAPI MyThread(LPTHREAD_START_ROUTINE lpStartAddress,
					   LPDWORD lpThreadId,
					   LPVOID lpParameter = NULL,
					   DWORD dwCreationFlags = 0);
HANDLE MyOpenFile(
                LPCTSTR lpszFilename,
                DWORD dwDesiredAccess, 
                DWORD dwShareMode);
HRESULT GenerateConfigTable();
void my_soap_init(struct soap *pSoap);
void SoapErr(struct soap *soap);
LPCTSTR lpszCfgFile = _T("./test.ini");
PCSTR pszCfgFile = "./test.ini";
HANDLE hSoapServerThd = NULL;
DWORD soapSvrThdid;
BOOL startSvr = false;

void SoapSendMyStack(struct soap *pSoap, poutStackBuffer pStack)
{
	if (pStack->index > 0)
    {
		//printf("%s", pStack->pBuffer);
		soap_send_raw(pSoap, pStack->pBuffer, pStack->index);
		emptyzeroStack(pStack);
    }
}

void GenerateConfigTableCRT(struct soap *pSoap, poutStackBuffer pStack)
{
    const int BUFFER_SIZE = 128;
    FILE *pCfgFile = OpenWebFileCRT("config.ini");
    if (pCfgFile == NULL) 
    {
		ShowInfo("file not found!");
		return;
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
        dwBytesRead = ReadFileBytesCRT(pCfgFile, lpBuffer, BUFFER_SIZE);
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
		SoapSendMyStack(pSoap, pStack);
    } while (!(dwBytesRead < BUFFER_SIZE));
    pushStack(pStack, "</table>", strlen("</table>"));
    
    if (lpBuffer) 
    {
        delete[] lpBuffer;
        lpBuffer = NULL;
    }
    fclose(pCfgFile);
}

void GetHtml(struct soap *pSoap, FILE *pfHtml)
{
    if (pfHtml == NULL) 
    {
        ShowInfo("get file error!");
        return;
    }
    const int BUFFER_SIZE = 1024 * 8;
    char read_buf[BUFFER_SIZE] = {0};
    outStackBuffer bufStack;
	initStack(&bufStack, BUFFER_SIZE * 2);

    //ShowInfo("get file success!");
    DWORD dwBytesRead = 0;
    int status = 0;
    char lastCh = '\0';
	int pushed = -1;
    do {
        int start = -1;
        int end = -1;
        ZeroMemory(read_buf, BUFFER_SIZE);
        dwBytesRead = ReadFileBytesCRT(pfHtml, read_buf, BUFFER_SIZE);
        // search for <% %> 不能嵌套使用
        for (int i = 0; i < dwBytesRead; i++)
        {
			char ch = *(read_buf + i);
            switch (ch)
            {
				case '\t':
                case '\r':
                case '\n':
				    break;
			    case '%':
			    {
				    if (status == 0 && lastCh == '<')
				    {
				        popStack(&bufStack);
						GenerateConfigTableCRT(pSoap, &bufStack);
				        status++;
				    }
			    }
                case '>':
                {
				    if (status == 1 && lastCh == '%')
				    {
				        status--;
			            break;
				    }
                }
			    default:
                {
					if (status == 0)
					{
						pushStack(&bufStack, ch);
					}
                }
            }
			lastCh = ch;
        }
		SoapSendMyStack(pSoap, &bufStack);
    } while (!(dwBytesRead < BUFFER_SIZE));
	deleteStack(&bufStack);
}

void AllTests()
{
    //gethtmltest();
}

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
	                            _T("./test.ini"), 
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

void SoapErr(struct soap *soap)
{
    WriteLog(NULL);
    soap_print_fault(soap, stderr);
}

HANDLE SelectFile(struct soap *soap, FileType ft)
{
	HANDLE hFile;
	switch (ft)
    {
        // HTTP response header with text/html
        case HTML:
		{
		    soap->http_content = "text/html; charset=gb2312";
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
		    soap->http_content = "application/x-javascript; charset=UTF-8";
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
		    soap->http_content = "text/css";
			hFile = OpenWebFile(_T("./main.css"));
            break;
		}
        default:
		{
		    soap->http_content = "text/xml; charset=UTF-8";
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
    
	soap_response(soap, SOAP_FILE);
	//ShowInfo(soap->path);
	FileType ft = GetFileType(soap->path);
	// GetFullFilePath here.
	hFile = SelectFile(soap, ft);

    if (hFile == INVALID_HANDLE_VALUE) 
    {
        ShowInfo("get file error!");
        hr = HRESULT_FROM_WIN32(::GetLastError());
    }
    if (SUCCEEDED(hr))
    {
        //ShowInfo("get file success!", strlen("get file success!"));
        do {
            int start = -1;
            int end = -1;
            ReadFileToBuffer(hFile,
                read_buf, 
                BUFFER_SIZE, 
                &dwBytesRead);

            if (ft == HTML)
            {
				CloseHandle(hFile);
				// 若以Unicode字符的方式打开的文件，必须重新打开，
				// 否则读不出数据。
				hFile = OpenWebFileA("index.htm");
				int hCrt = _open_osfhandle((intptr_t)hFile, _O_RDONLY | _O_TEXT);
				FILE *hfHtml = _fdopen(hCrt, "rt");
				if (hfHtml != NULL)
				{
					GetHtml(soap, hfHtml);
				}
				//这里不需要关闭文件
				//请求完成时会调用CloseHandle，如果这里调用
				//fclose(hfHtml);
				//再调用CloseHandle会出错。
				//所以这里不用关闭文件
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

    return SOAP_OK;
}

void MyCloseSoap(struct soap *psoap)
{
    soap_destroy(psoap);
    soap_end(psoap);
    soap_done(psoap);
    free(psoap);
}

DWORD WINAPI ProcessRequest(LPVOID lpThreadParam)
{
    struct soap *psoap = (struct soap *)lpThreadParam;
    if (soap_serve(psoap) != SOAP_OK) //
    {
        MessageBox(0, _T("soap_serve Error!"), _T("Error"), MB_OK);
    }
	MyCloseSoap(psoap);
    return 0;
}

#define BACKLOG (100) // Max. request backlog
#define MAX_THR (3) // Max. threads to serve requests
#define MAX_QUEUE (100) // Max. size of request queue

SOAP_SOCKET queue[MAX_QUEUE]; // The global request of sockets
int head = 0, tail = 0; // Queue head and tail


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
	pSoap->fget = MyHttpGet;//CRT;
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
