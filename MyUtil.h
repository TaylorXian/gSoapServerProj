extern LPCSTR pszLogFile;
extern LPCSTR pszHomeHtml;
extern LPCSTR pszSoapXml;
extern LPCSTR lpszFormat;

typedef enum {
    HTML, JS, CSS, INI, LOG, OTHER
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

FileType GetFileFullPath(LPSTR lpszFullpath, LPCSTR path)
{
    if (path)
    {
        if (!strcmp(path, "/"))
        {
            sprintf(lpszFullpath, lpszFormat, pszHomeHtml);
            return HTML;
        }
        if (strstr(path, ".htm"))
        {
            if (strstr(path, "?"))
            {
                sprintf(lpszFullpath, lpszFormat, pszHomeHtml);
            }
            else
            {
                sprintf(lpszFullpath, lpszFormat, path);
            }
            return HTML;
        }
        if (strstr(path, ".js")) 
        {
            sprintf(lpszFullpath, lpszFormat, path);
            return JS;
        }
        if (strstr(path, ".css"))
        {
            sprintf(lpszFullpath, lpszFormat, path);
            return CSS;
        }
        if (strstr(path, ".log"))
        {
            sprintf(lpszFullpath, lpszFormat, path);
            return LOG;
        }
        if (strstr(path, ".ini"))
        {
            sprintf(lpszFullpath, lpszFormat, path);
            return INI;
        }
        sprintf(lpszFullpath, lpszFormat, pszSoapXml);
    }
    return OTHER;
}

DWORD GetFileCurrentPointer(HANDLE hFile)
{
    return SetFilePointer(hFile, 0, NULL, FILE_CURRENT);
}
BOOL MySetFileEnd(HANDLE hFile, int oldDataEnd)
{
	int newEndFile = GetFileCurrentPointer(hFile);
	if (newEndFile < oldDataEnd)
	{
		return SetEndOfFile(hFile);
	}
	return true;
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
    char logpath[64] = {0};
    GetFileFullPath(logpath, pszLogFile);
    sprintf(time_str, "%4d/%2d/%2d %2d:%2d:%2d %4dms ... ", timeNow.wYear, timeNow.wMonth, timeNow.wDay,
        timeNow.wHour, timeNow.wMinute, timeNow.wSecond, timeNow.wMilliseconds);
    //strftime(time_str, sizeof(time_str), "%Y/%m/%d %H:%M:%S %z...", localtime(&t));
	pfLog = fopen(logpath, "a+");
	fprintf(pfLog, "%s", time_str);
	vfprintf(pfLog, info_format, arg_ptr);
	fprintf(pfLog, "\n");
    fclose(pfLog);
	va_end(arg_ptr);
}

FILE* OpenFileReadCRT(LPCSTR lpFilename)
{
    if (strlen(lpFilename) > 0)
    {
        return fopen(lpFilename, "rt"); // read text file.
    }
    return NULL;
}
DWORD ReadFileBytesCRT(FILE* pf, void* pDstBuf, DWORD lengthBuf)
{
    return fread(pDstBuf, sizeof(char), lengthBuf, pf);
}
DWORD WriteFileBytesCRT(FILE *pf, const void* pSrcBuf, DWORD lengthBuf)
{
    return fwrite(pSrcBuf, sizeof(char), lengthBuf, pf);
}
int CloseFile(FILE* pf)
{
    if (pf == NULL)
    {
        return 0;
    }
    return fclose(pf);
}

#ifndef WINCE
HANDLE MyOpenFileA(
                LPCSTR lpszFilename,
                DWORD dwDesiredAccess, 
                DWORD dwShareMode)
{
    return CreateFileA(
                    lpszFilename, 
                    dwDesiredAccess,
                    dwShareMode,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
}

HANDLE OpenWebFileA(LPCSTR lpszFilename)
{
    return MyOpenFileA(lpszFilename, 
                    GENERIC_READ,
                    FILE_SHARE_READ);
}
#endif

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

FILE* OpenWebFileCRT(LPCSTR lpszFilename)
{
    return OpenFileReadCRT(lpszFilename);
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
int MbToWc(LPCSTR lpMultiByteStr,
           int    cbMultiByte,
           LPWSTR lpWideCharStr,
           int    cchWideChar)
{
    return MultiByteToWideChar(CP_ACP, 0, lpMultiByteStr, cbMultiByte, lpWideCharStr, cchWideChar);
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

int ShowInfo(LPCSTR lpInfo, int lenInfo = -1)
{
    wchar_t *p = NULL;
    int lenWideChar = 0;
    if (lenInfo > 0)
    {
        lenWideChar = lenInfo;
    }
    else
    {
        lenWideChar = strlen(lpInfo);
    }
    p = new wchar_t[lenWideChar + 1];
    MbToWc(lpInfo, (lenInfo > 0 ? lenInfo : (-1)), p, lenWideChar + 1);
    int reVal = ::MessageBox(0, p, _T("info"), MB_OK);
    delete[] p;
    return reVal;
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

