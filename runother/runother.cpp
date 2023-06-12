#include <string>
#include <windows.h>
#include<io.h>
#include<thread>
#include<future>
DWORD pipid = 0;
#define MAX_NAME_LEN 2000

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
	WCHAR my_name[MAX_NAME_LEN];
	GetModuleFileNameW(NULL, my_name, ARRAYSIZE(my_name));
	my_name[wcslen(my_name) - 3] = L't'; my_name[wcslen(my_name) - 1] = L't';	//exe -> txt, com -> tot
	std::wstring wbuffer;
	FILE* f;
	_wfopen_s(&f, my_name, L"rt, ccs=UTF-8");
	if (f == NULL) return -1;
	long long filesize = _filelengthi64(_fileno(f));
	if (filesize < 0) { fclose(f); return -1; }

	WCHAR* s = GetCommandLine();
	if (*s == '"') {
		++s;
		while (*s)
			if (*s++ == '"') break;
	}
	else {
		while (*s && *s != ' ' && *s != '\t') ++s;
	}
	while (*s == ' ' || *s == '\t') s++;
	WCHAR* newcmd = new WCHAR[filesize + wcslen(s) + 1];
	size_t wchars_read = fread(newcmd, sizeof(wchar_t), filesize, f);
	newcmd[filesize] = 0;
	wcscat_s(newcmd, filesize + wcslen(s) + 1, s);
	fclose(f);

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
	CreateProcess(NULL, newcmd, NULL, NULL, TRUE, CREATE_NEW_PROCESS_GROUP, NULL, NULL, &si, &pi);
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
	delete[] newcmd;
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return exitCode;
}
