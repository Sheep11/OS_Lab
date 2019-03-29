#include<stdio.h>
#include<stdlib.h>
#include<Windows.h>

#define MUTEX_NAME "mutex"
#define SEM_FULL_NAME "sem_full"
#define SEM_EMPTY_NAME "sem_empty"
#define SHM_BUFFER_NAME "share_buffer"
#define BUFFER_SIZE 3
#define PRODUCER_NUM 2
#define CONSUMER_NUM 4

struct buffer_t {
	int cursor;
	int buffer[3];
};

void Producer() {
	//open handles
	HANDLE hmutex = OpenMutex(FILE_MAP_ALL_ACCESS, FALSE, MUTEX_NAME);
	HANDLE hfull = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, SEM_FULL_NAME);
	HANDLE hempty = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, SEM_EMPTY_NAME);
	HANDLE hmapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, SHM_BUFFER_NAME);
	buffer_t *pbuffer = (buffer_t*)MapViewOfFile(hmapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	//printf("%d %d %d %d", hmutex, hfull, hempty, hmapping);

	int pid = GetCurrentProcessId();
	printf("producer:%d begin \n", pid);

	//produce 4 times
	for (int i = 0; i < 4; i++) {
		Sleep(500);
		WaitForSingleObject(hempty, INFINITE);
		WaitForSingleObject(hmutex, INFINITE);

		int cursor = pbuffer->cursor;
		pbuffer->cursor++;
		pbuffer->buffer[cursor] = i;
		printf("producer(%d), put %d at %d\n", pid, i, cursor);

		ReleaseMutex(hmutex);
		ReleaseSemaphore(hfull, 1, NULL);
	}

	UnmapViewOfFile(pbuffer);
	CloseHandle(hmutex);
	CloseHandle(hfull);
	CloseHandle(hempty);
	CloseHandle(hmapping);
}

void Consumer() {
	//open handles
	HANDLE hmutex = OpenMutex(FILE_MAP_ALL_ACCESS, FALSE, MUTEX_NAME);
	HANDLE hfull = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, SEM_FULL_NAME);
	HANDLE hempty = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, SEM_EMPTY_NAME);
	HANDLE hmapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, SHM_BUFFER_NAME);
	buffer_t *pbuffer = (buffer_t*)MapViewOfFile(hmapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	int pid = GetCurrentProcessId();
	printf("consumer:%d begin\n", pid);

	//consume 2 times
	for (int i = 0; i < 2; i++) {
		Sleep(3000);
		WaitForSingleObject(hfull, INFINITE);
		WaitForSingleObject(hmutex, INFINITE);

		int cursor = pbuffer->cursor;
		pbuffer->cursor--;
		printf("consumer(%d), get %d at %d\n", pid, pbuffer->buffer[cursor], cursor - 1);

		ReleaseMutex(hmutex);
		ReleaseSemaphore(hempty, 1, NULL);
	}

	UnmapViewOfFile(pbuffer);
	CloseHandle(hmutex);
	CloseHandle(hfull);
	CloseHandle(hempty);
	CloseHandle(hmapping);
}

int main(int argv, char **argc) {
	//child process
	if (argv > 1) {
		if (strcmp(argc[1], "producer") == 0)
			Producer();
		else if (strcmp(argc[1], "consumer") == 0)
			Consumer();
		return 0;
	}

	//main process
	HANDLE hMutex= CreateMutex(NULL, FALSE, MUTEX_NAME);
	//ReleaseMutex(hMutex);

	HANDLE hFull = CreateSemaphore(NULL, 0, BUFFER_SIZE, SEM_FULL_NAME);
	HANDLE hEmpty = CreateSemaphore(NULL, BUFFER_SIZE, BUFFER_SIZE, SEM_EMPTY_NAME);

	//create share memory and init buffer
	HANDLE hMap = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		sizeof(buffer_t),
		SHM_BUFFER_NAME
	);
	buffer_t *pbuffer = (buffer_t*)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(buffer_t));
	pbuffer->cursor = 0;
	UnmapViewOfFile(pbuffer);
	//CloseHandle(hMap);

	//create producer
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(NULL, szFileName, MAX_PATH);

	TCHAR szCmdLine[MAX_PATH];
	sprintf(szCmdLine, "\"%s\" %s", szFileName, "producer");

	for (int i = 0; i < PRODUCER_NUM; i++) {
		STARTUPINFO si;
		ZeroMemory(reinterpret_cast<void*>(&si), sizeof(si));
		si.cb = sizeof(si);
		PROCESS_INFORMATION pi;

		bool bCreateOK = CreateProcess(
			szFileName,
			szCmdLine,
			NULL, NULL, FALSE, 0, NULL, NULL,
			&si, &pi
		);
		if (bCreateOK) {
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
	}

	//create consumer
	sprintf(szCmdLine, "\"%s\" %s", szFileName, "consumer");
	for (int i = 0; i < CONSUMER_NUM; i++) {
		STARTUPINFO si;
		ZeroMemory(reinterpret_cast<void*>(&si), sizeof(si));
		si.cb = sizeof(si);
		PROCESS_INFORMATION pi;

		bool bCreateOK = CreateProcess(
			szFileName,
			szCmdLine,
			NULL, NULL, FALSE, 0, NULL, NULL,
			&si, &pi
		);
		if (bCreateOK) {
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
	}


	return 0;
}