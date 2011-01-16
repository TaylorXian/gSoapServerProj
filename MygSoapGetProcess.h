FILE* SelectFileCRT(struct soap *soap, FileType ft)
{
	FILE *pFile;
	switch (ft)
    {
        // HTTP response header with text/html
        case HTML:
		{
            if (strstr(soap->path, "test"))
            {
                pFile = OpenReadCRT("./test.htm");
            }
            else
            {
                pFile = OpenReadCRT("./index.htm");
            }
            break;
		}
		case JS:
		{
		    if (strstr(soap->path, "main"))
		    {
                pFile = OpenReadCRT("./main.js");
		    }
		    else
		    {
                pFile = OpenReadCRT("./jquery-1.4.4.min.js");
            }
            break;
		}
        case CSS: 
		{
			pFile = OpenReadCRT("./main.css");
            break;
		}
        default:
		{
			if (rand() % 2 + 1)
			{
				pFile = OpenReadCRT("./ns.winconfig.req.xml");
			}
			else
			{
				pFile = OpenReadCRT("./ns.add.req.xml");
			}
		}
    }

	return pFile;
}

HRESULT SendConfigTableCRT(struct soap* pSoap, poutStackBuffer pStack)
{
    HRESULT hr = S_OK;
    const int BUFFER_SIZE = 128;
    FILE *pCfgFile = OpenReadCRT(pszCfgFile);
    if (pCfgFile == NULL) 
    {
        return E_FAIL;
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
        dwBytesRead = fread(lpBuffer, sizeof(char), BUFFER_SIZE, pCfgFile);
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
    fclose(pCfgFile);
    return hr;
}

int MyHttpGetCRT(struct soap *soap)
{
    const int BUFFER_SIZE = 1024 * 8;
    HRESULT hr = S_OK;
    DWORD dwBytesRead = 0;
    char read_buf[BUFFER_SIZE] = {0};
    outStackBuffer bufStack;
	initStack(&bufStack, BUFFER_SIZE * 2);
    
	soap_response(soap, SOAP_FILE);
	soap->http_content = "text/html; charset=gb2312";
	ShowInfo(soap->path);
	FileType ft = GetFileType(soap->path);
    //hfile = selectfile(soap, ft);
    //if (hfile == invalid_handle_value) 
    //{
    //    showinfo("get file error!");
    //    hr = hresult_from_win32(::getlasterror());
    //}
	FILE *p = SelectFileCRT(soap, ft);
	
    if (p != NULL)
    {
        ShowInfo("get file success!");
        int status = 0;
		int pushed = -1;
        do {
            int start = -1;
            int end = -1;
            dwBytesRead = fread(read_buf, sizeof(char), BUFFER_SIZE,p); 
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
							SendConfigTableCRT(soap, &bufStack);
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
    fclose(p);
	WriteLog("MyHttpGet %s", soap->path);
	deleteStack(&bufStack);

    return SOAP_OK;
}
