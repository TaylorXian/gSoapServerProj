#include "soapH.h"
#include "ns.nsmap"
#include "outStackBuffer.h"
#include "MyUtil.h"
#include "MyFileParser.h"
#include "alltests.h"

LPCSTR pszCfgFile = "/test.ini";
LPCSTR pszLogFile = "/soap.log";
LPCSTR pszHomeHtml = "/index.htm";
LPCSTR pszSoapXml = "/ns.winconfig.req.xml";
HANDLE hSoapServerThd = NULL;
DWORD soapSvrThdid;
BOOL startSvr = false;

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
    char fullpath[64] = {0};
    GetFileFullPath(fullpath, pszCfgFile);
    FILE *pCfgFile = OpenWebFileCRT(fullpath);
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
	    HTTP_SVR_PORT, // 端口
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
	char filename[64] = {0};
	GetFileFullPath(filename, pszCfgFile);
	int len = strlen(filename) + 1;
	wchar_t* szFullpath = new wchar_t[len];
	MbToWc(filename, -1, szFullpath, len);

	HANDLE hCfgFile = MyOpenFile(
	                            szFullpath, 
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE);
    if (hCfgFile == INVALID_HANDLE_VALUE)
    {
        result = false;
        return HRESULT_FROM_WIN32(GetLastError());
    }
	delete[] szFullpath;
    DWORD FindKeyLite(HANDLE, LPSTR, LPSTR);
    DWORD pFile = FindKeyLite(hCfgFile, key, value);
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
    pushStack(&stack, '=');
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
	if (!MySetFileEnd(hCfgFile, rStart))
	{
		return 0;
	}
    delete[] pBuffer;
    deleteStack(&stack);
    return dwBytesWritten;
}

void SoapErr(struct soap *soap)
{
    WriteLog(NULL);
    soap_print_fault(soap, stderr);
}

VOID MIMEType4FileType(struct soap *soap, FileType ft)
{
	switch (ft)
    {
        case HTML:
            // HTTP response header with text/html
		    soap->http_content = "text/html; charset=gb2312";
            break;
		case JS:
		    soap->http_content = "application/x-javascript;";
            break;
        case CSS: 
		    soap->http_content = "text/css";
            break;
        default:
		    soap->http_content = "text/*; charset=UTF-8";
    }
}

HANDLE SelectFile(LPCSTR lpszFilename)
{
	HANDLE hFile;
	// GetFullFilePath here.
	int len = strlen(lpszFilename) + 1;
	wchar_t* szFullpath = new wchar_t[len];
	MbToWc(lpszFilename, -1, szFullpath, len);
	hFile = OpenWebFile(szFullpath);
	delete[] szFullpath;

	return hFile;
}

int MyHttpGet(struct soap *soap)
{
    const int BUFFER_SIZE = 1024 * 8;
    HRESULT hr = S_OK;
    HANDLE hFile;
    DWORD dwBytesRead = 0;
    char read_buf[BUFFER_SIZE] = {0};
    char szFilename[64] = {0};

	soap_response(soap, SOAP_FILE);
	FileType ft = GetFileFullPath(szFilename, soap->path);
	MIMEType4FileType(soap, ft);
	hFile = SelectFile(szFilename);
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

DWORD FindKeyLite(HANDLE hCfgFile, LPSTR key, LPSTR val)
{
    int ConfigChangeState(int*, char*);
    const int BUFFER_SIZE = 128;
    outStackBuffer stack;
    initStack(&stack, (strlen(key) + strlen(val) + BUFFER_SIZE) * 2);
    LPSTR pBuffer = new CHAR[BUFFER_SIZE];
    LPSTR pKey = key;
    DWORD dwBytesRead = 0;
    DWORD dwBytesWritten = 0;
    int i = 0;
    int rStart = -1;
    int wStart = -1;
    int status = 0;
    //SetFilePointer();
    pushStack(&stack, key, strlen(key));
    pushStack(&stack, '=');
    pushStack(&stack, val, strlen(val));
    pushStack(&stack, '\r');
    pushStack(&stack, '\n');
    do {
        i = 0;
        dwBytesRead = 0;
        dwBytesWritten = 0;
        ZeroMemory(pBuffer, BUFFER_SIZE);
        if (ReadFile(hCfgFile, pBuffer, BUFFER_SIZE - 1, &dwBytesRead, NULL))
        {
            // WriteLog("[read  %dB]%s", dwBytesRead, pBuffer);
            rStart = GetFileCurrentPointer(hCfgFile);
            if (status >= 3 && stack.index > 0 && (wStart + stack.index < rStart))
            {
                SetFilePointer(hCfgFile, wStart, NULL, FILE_BEGIN);
                if (WriteFile(hCfgFile, stack.pBuffer, stack.index, &dwBytesWritten, NULL))
                {
                    wStart += dwBytesWritten;
                    emptyStack(&stack);
                }
                SetFilePointer(hCfgFile, rStart, NULL, FILE_BEGIN);
            }
            for (i = 0; i < dwBytesRead; i++)
            {
                char ch = *(pBuffer + i);
                switch(status)
                {
					case 0:
                    {
                        wStart = GetFileCurrentPointer(hCfgFile) - dwBytesRead + i;
                    }
                    case 1:
                    {
                        if (ch == *pKey)
                        {
                            pKey++;
                            if (*pKey == '\0')
                            {
                                status = 2;
                            }
                            else
                            {
                                status = 1;
                            }
                        }
                        else
                        {
                            status = -1;
                        }
                        break;
                    }
                    case 2:
                    {
                        if (ch == ' ' || ch == '\t')
                        {
                        }
                        else if (ch == '=' || ch == '\r' || ch == '\n')
                        {
                            status = 3;
                        }
                        else
                        {
                            status = -1;
                        }
                        break;
                    }
                    case 3:
                    {
                        if (ch == '\n')
                        {
                            status = 4;
                        }
                        break;
                    }
                    case 4:
                    {
                        pushStack(&stack, ch);
                        break;
                    }
                    case -1:
                    {
                        wStart = -1;
                        pKey = key;
                        if (ch == '\n')
                        {
                            status = 0;
                        }
                    }
                    default:
                    {
                        break;
                    }
                }
            }
        }
    } while (!(dwBytesRead < BUFFER_SIZE - 1));
    if (stack.index > 0)
    {
        if (status >= 3 && wStart > -1)
        {
            SetFilePointer(hCfgFile, wStart, NULL, FILE_BEGIN);
        }
        if (WriteFile(hCfgFile, stack.pBuffer, stack.index, &dwBytesWritten, NULL))
        {
            wStart += dwBytesWritten;
            emptyStack(&stack);
        }
    }
    delete[] pBuffer;
    deleteStack(&stack);
	if (!MySetFileEnd(hCfgFile, rStart))
	{
		return 0;
	}
    return dwBytesWritten;
}
