#include<iostream>
#include<stdlib.h>
#include<Windows.h>
#include<iomanip>
#include<Shlwapi.h>
#include<Psapi.h>
#pragma comment(lib,"Shlwapi.lib")
using namespace std;

inline bool TestSet(DWORD dwTarget, DWORD dwMask) {
	return ((dwTarget & dwMask) == dwMask);
}

#define SHOWMASK(dwTarget,type)\
if(TestSet(dwTarget,PAGE_##type))\
	{cout<<","<<#type;}

void ShowProtection(DWORD dwTarget) {
	SHOWMASK(dwTarget, READONLY);
	SHOWMASK(dwTarget, GUARD);
	SHOWMASK(dwTarget, NOCACHE);
	SHOWMASK(dwTarget, READWRITE);
	SHOWMASK(dwTarget, WRITECOPY);
	SHOWMASK(dwTarget, EXECUTE);
	SHOWMASK(dwTarget, EXECUTE_READ);
	SHOWMASK(dwTarget, EXECUTE_WRITECOPY);
	SHOWMASK(dwTarget, EXECUTE_READWRITE);
	SHOWMASK(dwTarget, NOACCESS);
}

SYSTEM_INFO si;
MEMORYSTATUSEX ms;
PERFORMACE_INFORMATION pi;
HANDLE hprocess;

void PrintMemoryInfo() {
	cout << "------- System Memory Info -------" << endl
		<< "物理内存占用率：" << (float)ms.dwMemoryLoad << "%" << endl
		<< fixed << setprecision(2)
		<< "总物理内存：" << (float)ms.ullTotalPhys / 1024 / 1024 / 1024 << "GB" << endl
		<< "可用物理内存：" << (float)ms.ullAvailPhys / 1024 / 1024 / 1024 << "GB" << endl
		<< "总虚拟内存：" << (float)ms.ullTotalVirtual / 1024 / 1024 / 1024 << "GB" << endl
		<< "可用虚拟内存：" << (float)ms.ullAvailVirtual / 1024 / 1024 / 1024 << "GB" << endl;

	cout << "------- System Performance Info -------" << endl
		<< "总物理页：" << pi.CommitTotal << endl
		<< "可用物理页：" << pi.PhysicalAvailable << endl
		<< "系统缓存页：" << pi.SystemCache << endl
		<< "内核池中分页与非分页内存总数：" << pi.KernelTotal << endl
		<< "页大小：" << pi.PageSize << endl
		<< "句柄数：" << pi.HandleCount << endl
		<< "进程数：" << pi.ProcessCount << endl
		<< "线程数：" << pi.ThreadCount << endl;

	cout << "------- System Info -------" << endl
		<< "CPU数量：" << si.dwNumberOfProcessors << endl
		<< hex << setw(8)
		<< "程序可访问最低内存：" << si.lpMinimumApplicationAddress << endl
		<< "程序可访问最高内存：" << si.lpMaximumApplicationAddress << endl;


	cout << "------- Virtual Memory -------" << endl;
	LPCVOID pBlock = (LPVOID)si.lpMinimumApplicationAddress;
	MEMORY_BASIC_INFORMATION mbi;
	ZeroMemory(&mbi, sizeof(mbi));
	while (pBlock < si.lpMaximumApplicationAddress) {
		if (VirtualQueryEx(hprocess, pBlock, &mbi, sizeof(mbi)) == sizeof(mbi)) {
			LPCVOID pEnd = (PBYTE)pBlock + mbi.RegionSize;
			TCHAR szSize[MAX_PATH];
			StrFormatByteSize(mbi.RegionSize, szSize, MAX_PATH);

			cout.fill('0');
			cout << hex << setw(8) << (DWORD)pBlock
				<< "-"
				<< hex << setw(8) << (DWORD)pEnd
				<< "(" << szSize << ") ";

			//show block state
			switch (mbi.State) {
			case MEM_COMMIT:
				printf("Committed");
				break;
			case MEM_FREE:
				printf("Free");
				break;
			case MEM_RESERVE:
				printf("Reserve");
				break;
			}

			//show protect info
			if (mbi.Protect == 0 && mbi.State != MEM_FREE) {
				mbi.Protect = PAGE_READONLY;
			}
			ShowProtection(mbi.Protect);

			//show type
			switch (mbi.Type) {
			case MEM_IMAGE:
				printf(",Image");
				break;
			case MEM_MAPPED:
				printf(",Mapped");
				break;
			case MEM_PRIVATE:
				printf(",Private");
				break;
			}

			//show executable images
			TCHAR szFilename[MAX_PATH];
			if (GetModuleFileName((HMODULE)pBlock, szFilename, MAX_PATH) > 0) {
				PathStripPath(szFilename);
				printf(",Module:%s", szFilename);
			}
			printf("\n");

			pBlock = pEnd;
		}
	}

}

int main() {
	hprocess = GetCurrentProcess();

	//performance info
	GetPerformanceInfo(&pi, sizeof(pi));

	//virtual memory info
	ms.dwLength = sizeof(ms);
	GlobalMemoryStatusEx(&ms);
	
	//system info
	ZeroMemory(&si, sizeof(si));
	GetSystemInfo(&si);
	
	//start print
	PrintMemoryInfo();

	return 0;
}