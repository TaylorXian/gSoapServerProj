//pthread_mutex_t queue_cs;
//pthread_cond_t queue_cv;

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