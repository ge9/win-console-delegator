#include <string>
#include <windows.h>
#include<io.h>
#define MAX_NAME_LEN 2000

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPrevInst,
    LPWSTR lpCmdLine, int intShowCmd)
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

	WCHAR* newcmd = new WCHAR[filesize + wcslen(lpCmdLine) + 1];
	size_t wchars_read = fread(newcmd, sizeof(wchar_t), filesize, f);
	newcmd[filesize] = 0;
	wcscat_s(newcmd, filesize + wcslen(lpCmdLine) + 1, lpCmdLine);
	fclose(f);

	//prepare STARTUPINFO
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(STARTUPINFO);

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));
	CreateProcess(NULL, newcmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);

	DWORD r = WaitForSingleObject(pi.hProcess, INFINITE);
	switch (r) {
	case WAIT_OBJECT_0: break;
	default: return -1;
	}
	DWORD exitCode;
	if (!GetExitCodeProcess(pi.hProcess, &exitCode))  return -1;
	delete[] newcmd;
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return exitCode;
}