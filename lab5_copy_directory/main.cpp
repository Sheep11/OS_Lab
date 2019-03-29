#include<stdlib.h>
#include<Windows.h>
#include<iostream>

using namespace std;

int main(int argv, char **argc) {
	HANDLE hfile = CreateFile(
		argc[1], GENERIC_READ,
		NULL, NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	HANDLE hnewfile = CreateFile(
		argc[2],GENERIC_READ | GENERIC_WRITE,
		NULL,NULL,
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

	return 0;
}