
#ifdef WINCE
void WinApiTest()
{
    ShowInfo("ready to Test!");
    char buf[1024] = {0};
    DWORD dwRead = 0;
    ShowInfo("ready to OpenFile!");
    HANDLE hFile = OpenWebFile(_T("/PocketMory1/index.htm"));
    if (hFile == INVALID_HANDLE_VALUE)
    {
        ShowInfo("OpenFile error!");
    }
    else
    {
        ShowInfo("OpenFile!");
        HRESULT hr = ReadFileToBuffer(hFile, buf, 1024 - 1, &dwRead);
        if (SUCCEEDED(hr))
        {
            ShowInfo(buf);
            // 可以读出文件，没有问题。
        }
        else
        {
            ShowInfo("Read error!");
        }
    }
    CloseHandle(hFile);
}
#else

//#pragma comment(lib, "../gtest-1.5.0/gtestd.lib")
#include <gtest/gtest.h>

TEST(strstrtest, urlpath)
{
    int const TEST_PATH_LEN = 64;
    char urlpath[64] = "http://localhost:18083/index.htm?file=/PocketMory1/config.ini";
    EXPECT_EQ(3, strlen(urlpath));
    EXPECT_EQ(reinterpret_cast<const char *>(NULL), strstr("/index.htm?", "?"));

    EXPECT_LT(reinterpret_cast<const char *>(NULL), strstr(urlpath, "?"));
    printf("Sub String %s\n", strstr(urlpath, "?"));
    //printf(strstr(urlpath, "?"));
    printf("\n");
    char urlpathtest[TEST_PATH_LEN] = "/index.htm?file=/PocketMory1/config.ini";
    char fullpath[TEST_PATH_LEN] = {0};
    GetFileFullPath(fullpath, urlpathtest);
    printf("testcase: %s", urlpathtest);
    printf("\n");
    printf("result:   %s", fullpath);
    printf("\n");
    ZeroMemory(fullpath, TEST_PATH_LEN);
    ZeroMemory(urlpathtest, TEST_PATH_LEN);
    sprintf(urlpathtest, "%s", "/index1.htm?file=/PocketMory1/config.ini");
    GetFileFullPath(fullpath, urlpathtest);
    printf("testcase: %s", urlpathtest);
    printf("\n");
    printf("result:   %s", fullpath);
    printf("\n");
}

int MyGoogleTest()
{
    int argc = 0;
    LPTSTR argv = NULL;
    testing::InitGoogleTest(&argc, &argv);
    return RUN_ALL_TESTS();
}

BOOL PrintStrings(HANDLE hOut, ...)
{
    BOOL reVal = TRUE;
    DWORD lenMsg;
    DWORD count;
    LPCTSTR pMsg;
    /* Current message string. */
    va_list pMsgList;
    /* Start processing messages. */
    va_start (pMsgList, hOut);
    while ((pMsg = va_arg(pMsgList, LPCTSTR)) != NULL) {
        lenMsg = _tclen(pMsg);
        /* WriteConsole succeeds only for console handles. */
        if (!WriteConsole(hOut, pMsg, lenMsg, &count, NULL) &&
            /* Call WriteFile only if WriteConsole fails. */
            !WriteFile(hOut, pMsg, lenMsg * sizeof(TCHAR), &count, NULL))
        {
            reVal = FALSE;
            break;
        }
    }
    va_end(pMsgList);
    return reVal;
}

/* Single message version of PrintStrings. */
BOOL PrintMsg(HANDLE hOut, LPCTSTR pMsg)
{
    return PrintStrings(hOut, pMsg, NULL);
}
HANDLE WINAPI OpenConsole(
    LPCTSTR lpConsoleName,
    DWORD dwDesiredAccess)
{
    return CreateFile(lpConsoleName, 
        dwDesiredAccess, 
        0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
}
/* call AllocConsole() first, otherwise return INVALID_HANDLE_VALUE. */
HANDLE WINAPI OpenStdIn()
{
    return OpenConsole(_T("CONIN$"), GENERIC_READ | GENERIC_WRITE);
}
/* call AllocConsole() first, otherwise return INVALID_HANDLE_VALUE. */
HANDLE WINAPI OpenStdOut()
{
    return OpenConsole(_T("CONOUT$"), GENERIC_WRITE);
}
/* Prompt the user at the console and get a response. */
BOOL ConsolePrompt(LPCTSTR pPromptMsg, 
    LPTSTR pResponse, 
    DWORD maxTChar, 
    BOOL echo)
{
    HANDLE hStdIn;
    HANDLE hStdOut;
    DWORD inTChar;
    DWORD echoFlag;
    BOOL success;
    hStdIn = OpenStdIn();
    hStdOut = OpenStdOut();
    echoFlag = echo ? ENABLE_ECHO_INPUT : 0;
    success = SetConsoleMode(hStdIn, 
            ENABLE_LINE_INPUT | echoFlag | ENABLE_PROCESSED_INPUT) &&
        SetConsoleMode(hStdOut, 
            ENABLE_WRAP_AT_EOL_OUTPUT | ENABLE_PROCESSED_OUTPUT) &&
        PrintStrings(hStdOut, pPromptMsg, NULL) &&
        ReadConsole(hStdOut, pResponse, maxTChar, &inTChar, NULL);
    if (success) 
    {
        pResponse[inTChar - 2] = '\0';
    }
    CloseHandle(hStdIn);
    CloseHandle(hStdOut);
    return success;
}

/* General-purpose function for reporting system errors. */
VOID ReportError(LPCTSTR userMessage, DWORD exitCode, 
    BOOL printErrorMsg)
{
    DWORD lenErrorMsg;
    DWORD lastErr = GetLastError();
    LPCTSTR lpvSysMsg;
    HANDLE hStdErr = GetStdHandle(STD_ERROR_HANDLE);
    PrintMsg(hStdErr, userMessage);
    if (printErrorMsg) 
    {
        lenErrorMsg = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 
            NULL, lastErr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
            (LPTSTR) &lpvSysMsg, 0, NULL);
        PrintStrings(hStdErr, _T("\n"), lpvSysMsg, _T("\n"), NULL);
        /* Free the memory block containing the error message. */
        HeapFree(GetProcessHeap(), 0, (LPVOID)lpvSysMsg);
    }
    if (exitCode > 0)
    {
        ExitProcess(exitCode);
    }
    else
    {
        return;
    }
}
VOID MyWriteConsole()
{
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    LPCSTR pMsg = "this is console test.";
    DWORD dwWritten;
    if (WriteConsole(hStdOut, pMsg, strlen(pMsg) / 2, &dwWritten, NULL))
    {
        ShowInfo(pMsg);
    }
    CloseHandle(hStdOut);
}
VOID OpenStdConsoleCRT()
{
    _tfreopen(_T("CONOUT$"), _T("w+t"), stdout);
    _tfreopen(_T("CONIN$"), _T("w+t"), stdin);
    //printf("this is test.");
    //printf("this is test.");
    /***********************************************
    用这种方式也可以将控制台与crt的标准输入输出相关联
    intptr_t handle = (intptr_t)GetStdHandle(STD_OUTPUT_HANDLE);
    //Associates a C run-time file descriptor with an existing operating-system file handle.
    int hCrt = _open_osfhandle(handle, _O_TEXT);
    //Associates a stream with a file that was previously opened for low-level I/O.
    FILE *hf = _fdopen(hCrt, "w");
    *stdout = *hf;
    
    TCHAR title[64] = {0};
    SetConsoleTitle(title);
    SetConsoleTextAttribute((HANDLE)handle, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    ************************************************/
}
VOID CloseStdConsoleCRT()
{
    fclose(stdout);
    fclose(stdin);
}

void AllTests()
{
    //gethtmltest();
    MyGoogleTest();
}
#endif