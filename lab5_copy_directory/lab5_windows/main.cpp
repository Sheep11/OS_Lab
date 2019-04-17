#include<stdlib.h>
#include<tchar.h>
#include<Windows.h>
#include<AclAPI.h>
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
	
	PSECURITY_DESCRIPTOR psd;
	PSID powner, pgroup;
	PACL psacl, pdacl;
	DWORD MASK = OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION;

	GetSecurityInfo(hfile, SE_FILE_OBJECT, MASK, &powner,&pgroup,&pdacl,&psacl, &psd);
	SetSecurityInfo(hnewfile, SE_FILE_OBJECT, MASK, powner, pgroup, pdacl, psacl);

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

void modify_dir_date(char *oldfile, char *newfile) {
	HANDLE hfile = CreateFile(
		oldfile, GENERIC_READ|GENERIC_WRITE,
		NULL, NULL,
		OPEN_ALWAYS,
		FILE_FLAG_BACKUP_SEMANTICS,
		NULL
	);

	HANDLE hnewfile = CreateFile(
		newfile, GENERIC_READ|GENERIC_WRITE,
		NULL, NULL,
		OPEN_ALWAYS,
		FILE_FLAG_BACKUP_SEMANTICS,
		NULL
	);
	FILETIME ftCreate, ftAccess, ftWrite;
	GetFileTime(hfile, &ftCreate, &ftAccess, &ftWrite);
	SetFileTime(hnewfile, &ftCreate, &ftAccess, &ftWrite);

	CloseHandle(hfile);
	CloseHandle(hnewfile);
}

int CopyDir(char *oldpath, char *newpath) {
	CreateDirectoryEx(oldpath,newpath,NULL);

	SetCurrentDirectory(oldpath);

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
	modify_dir_date(oldpath, newpath);
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

	SetCurrentDirectory("D:\\");
	CopyDir(szOldDir, szNewDir);

	return 0;
}