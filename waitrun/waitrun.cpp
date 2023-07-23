#include <string>
#include <windows.h>
#include<io.h>
#include<thread>
#include<future>
#include <conio.h>
#include "..\_header.h"
DWORD pipid = 0;

BOOL WINAPI HandlerRoutine(DWORD dwCtrlType)
{
	if (dwCtrlType == CTRL_C_EVENT) {
		GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, pipid);
		return TRUE;
	}
	return FALSE;
}

int main()
{
	WCHAR* s = GetCommandLine();
	s = removeExecPath(s);
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)HandlerRoutine, TRUE);

	//prepare STARTUPINFO
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));
	CreateProcess(NULL, s, NULL, NULL, TRUE, CREATE_NEW_PROCESS_GROUP, NULL, NULL, &si, &pi);
	pipid = pi.dwProcessId;

	DWORD r = WaitForSingleObject(pi.hProcess, INFINITE);
	switch (r) {
	case WAIT_OBJECT_0: break;
	default: printf("wait failed; exit code %d\n", r); return -1;
	}
	DWORD exitCode;
	if (!GetExitCodeProcess(pi.hProcess, &exitCode)) {
		printf("GetExitCodeProcess failed"); return -1;
	}
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	printf("waitrun: press some key to exit...\n");
	_getch();
	return exitCode;
}
