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
    HANDLE hCfgFile = OpenWebFile(_T("./test.ini"));
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

int ChangeStateCRT(int *pState, char *pCh)
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

