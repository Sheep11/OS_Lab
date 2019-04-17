#include<Windows.h>
#include<iostream>

using namespace std;

SYSTEMTIME time_span(SYSTEMTIME s, SYSTEMTIME e)
{
	SYSTEMTIME t;
	ZeroMemory(&t, sizeof(SYSTEMTIME));

	if (e.wMilliseconds < s.wMilliseconds)
	{
		t.wMilliseconds = e.wMilliseconds + 1000 - s.wMilliseconds;

		if (e.wSecond < 1)
		{
			e.wSecond += 59;
			e.wHour -= 1;
		}
		else
		{
			e.wSecond -= 1;
		}

		t.wSecond = e.wSecond - s.wSecond;
		t.wMinute = e.wMinute - s.wMinute;
	}
	else
	{
		t.wMilliseconds = e.wMilliseconds - s.wMilliseconds;

		if (e.wSecond < s.wSecond)
		{
			e.wSecond += 60;
			t.wSecond = e.wSecond - s.wSecond;

			if (e.wMinute < 1)
			{
				e.wHour -= 1;
				e.wMinute += 59;
			}
			else
			{
				e.wMinute -= 1;
			}
		}

		t.wSecond = e.wSecond - s.wSecond;
		t.wMinute = e.wMinute - s.wMinute;
	}

	t.wHour = e.wHour - s.wHour;
	t.wDay = e.wDay - s.wDay;
	t.wDayOfWeek = e.wDayOfWeek - s.wDayOfWeek;
	t.wYear = e.wYear - s.wYear;

	printf("Running time: %d hour %d minute %d second %d million second\n",
		t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);

	return t;
}

HANDLE fork(char* filename, char* arg = NULL) {
	cout << "child process begin, program name:" << filename << endl;
	if (arg != NULL)
		cout << "arg:" << arg << endl;

	//construct command line
	TCHAR cmd[MAX_PATH];
	if (arg == NULL)
		sprintf(cmd, "%s", filename);
	else
		sprintf(cmd, "%s %s", filename, arg);

	//create data struct for child process
	STARTUPINFO si;
	ZeroMemory(reinterpret_cast<void*>(&si), sizeof(si));
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi;

	bool create_ok = CreateProcess(
		NULL,
		cmd,
		NULL, NULL, FALSE,
		NULL,
		NULL, NULL,
		&si,
		&pi
	);

	//close handle
	if (create_ok) {
		//CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	else {
		printf("create process failed\n");
		return INVALID_HANDLE_VALUE;
	}
	return pi.hProcess;
}

int main(int argv, char **argc) {
	HANDLE hChild = NULL;
	if (argv == 1) {
		cout << "error" << endl << "mytime.exe [program] <time>";
		return 0;
	}
	else if (argv == 2)
		hChild = fork(argc[1]);	//child process without arg
	else
		hChild = fork(argc[1], argc[2]);	//child process with arg

	if (hChild == INVALID_HANDLE_VALUE)
		return 0;
		
	LPSYSTEMTIME start, end;
	start = (LPSYSTEMTIME)malloc(sizeof(start));
	end = (LPSYSTEMTIME)malloc(sizeof(start));

	//timing
	GetSystemTime(start);
	WaitForSingleObject(hChild, INFINITE);
	CloseHandle(hChild);
	GetSystemTime(end);

	//print time
	time_span(*start, *end);

	return 0;

}

