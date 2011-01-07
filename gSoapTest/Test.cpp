#include <stdio.h>
#include <tchar.h>
#include <windows.h>

int MyWriteFile(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);
	FILE *pf = fopen("test.txt", "a+");
	int n = vfprintf(pf, format, ap);
	fclose(pf);
	va_end(ap);

	return n;
}

int WINAPI _tWinMain(HINSTANCE hInstance,
			  HINSTANCE hPrevInstance,
			  LPTSTR lpCmdLine,
			  int nShowCmd)
{
	MyWriteFile("this is a test %d.%d", 1, 1);
	//_tprintf(_T("This is a test."));
	//printf("This is a test.");
	_tsystem(_T("pause"));
	return 0;
}