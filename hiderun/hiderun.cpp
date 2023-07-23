#include <windows.h>

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPrevInst,
    LPWSTR lpCmdLine, int intShowCmd) {
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi;

    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = TRUE;
    CreateProcess(NULL, lpCmdLine, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}