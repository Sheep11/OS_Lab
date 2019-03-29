#include<stdlib.h>
#include<tchar.h>
#include<Windows.h>
#include<iostream>

using namespace std;

int CopySingleFile(char *oldfile, char *newfile) {
	HANDLE hfile = CreateFile(
		oldfile, GENERIC_READ,
		NULL, NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	HANDLE hnewfile = CreateFile(
		newfile, GENERIC_READ | GENERIC_WRITE,
		NULL, NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		hfile
	);

	char buffer[MAX_PATH];
	DWORD dwread(0);
	DWORD dwwrite(0);
	do {
		ReadFile(
			hfile, buffer, sizeof(buffer),
			&dwread, NULL
		);
		WriteFile(
			hnewfile, buffer, dwread,
			&dwwrite, NULL
		);
	} while (sizeof(buffer) == dwread);

	FILETIME ftCreate, ftAccess, ftWrite;
	GetFileTime(hfile, &ftCreate, &ftAccess, &ftWrite);
	SetFileTime(hnewfile, &ftCreate, &ftAccess, &ftWrite);

	CloseHandle(hfile);
	CloseHandle(hnewfile);

	return 0;
}

int CopyDir(char *oldpath, char *newpath) {
	SetCurrentDirectory(oldpath);
	CreateDirectory(newpath, NULL);
	
	WIN32_FIND_DATA finddata;
	HANDLE hfile= FindFirstFile(_TEXT("*.*"), &finddata);
	bool fOk = (hfile != INVALID_HANDLE_VALUE);

	char next_oldpath[MAX_PATH], next_newpath[MAX_PATH];
	while (fOk) {
		if(strcmp(finddata.cFileName,".")==0||strcmp(finddata.cFileName,"..")==0){
			fOk = FindNextFile(hfile, &finddata);
			continue;
		}

		sprintf(next_oldpath, "%s\\%s", oldpath, finddata.cFileName);
		sprintf(next_newpath, "%s\\%s", newpath, finddata.cFileName);

		if (finddata.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) {
			CopyDir(next_oldpath, next_newpath);
			//SetCurrentDirectory(oldpath);
		}
		else {
			CopySingleFile(next_oldpath, next_newpath);
		}

		fOk = FindNextFile(hfile, &finddata);
	}

	SetCurrentDirectory("..\\");
	FindClose(hfile);
	return 0;
}

int main(int argc, char **argv) {
	TCHAR szRootDir[MAX_PATH], szOldDir[MAX_PATH], szNewDir[MAX_PATH];
	DWORD dwCurDir = GetCurrentDirectory(sizeof(szRootDir) / sizeof(TCHAR), szRootDir);
	
	SetCurrentDirectory(argv[1]);
	GetCurrentDirectory(sizeof(szOldDir) / sizeof(TCHAR), szOldDir);

	SetCurrentDirectory(szRootDir);

	if (!SetCurrentDirectory(argv[2]))
		CreateDirectory(argv[2],NULL);
	SetCurrentDirectory(argv[2]);
	GetCurrentDirectory(sizeof(szNewDir) / sizeof(TCHAR), szNewDir);

	CopyDir(szOldDir, szNewDir);

	return 0;
}