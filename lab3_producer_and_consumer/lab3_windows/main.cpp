#include<stdio.h>
#include<stdlib.h>
#include<Windows.h>
#include<time.h>

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

void show_time() {
	time_t now;
	time(&now);
	printf("%s", asctime(gmtime(&now)));
}

void show_buffer(buffer_t *pbuffer) {
	printf("now buffer: ");
	for (int i = 0; i < pbuffer->cursor; i++)
		printf("%d ", pbuffer->buffer[i]);
	printf("\n\n");
}

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

	srand(pid);
	//produce 4 times
	for (int i = 0; i < 4; i++) {
		Sleep(rand()%2000);
		WaitForSingleObject(hempty, INFINITE);
		WaitForSingleObject(hmutex, INFINITE);

		int cursor = pbuffer->cursor;
		pbuffer->cursor++;
		pbuffer->buffer[cursor] = rand()%100;

		show_time();
		printf("producer(%d), put %d at %d\n", pid, pbuffer->buffer[cursor], cursor);
		show_buffer(pbuffer);

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

	srand(pid);
	//consume 2 times
	for (int i = 0; i < 2; i++) {
		Sleep(rand() % 5000);
		WaitForSingleObject(hfull, INFINITE);
		WaitForSingleObject(hmutex, INFINITE);

		int cursor = pbuffer->cursor;
		pbuffer->cursor--;

		show_time();
		printf("consumer(%d), get %d at %d\n", pid, pbuffer->buffer[cursor - 1], cursor - 1);
		show_buffer(pbuffer);

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
	memset(pbuffer, 0, sizeof(buffer_t));
	UnmapViewOfFile(pbuffer);

	//childprocess handles
	HANDLE handles[CONSUMER_NUM + PRODUCER_NUM];
	int handle_count = 0;

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
			handles[handle_count++] = pi.hProcess;
			//CloseHandle(pi.hProcess);
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
			handles[handle_count++] = pi.hProcess;
			//CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
	}

	//close all handles
	WaitForMultipleObjects(handle_count, handles, TRUE, INFINITE);
	for(int i=0;i<handle_count;i++)
		CloseHandle(handles[i]);

	CloseHandle(hMutex);
	CloseHandle(hFull);
	CloseHandle(hEmpty);
	CloseHandle(hMap);

	system("pause");
	return 0;
}