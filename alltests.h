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

