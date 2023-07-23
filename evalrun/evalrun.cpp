#include <windows.h>
#include <iostream>
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

int main() {
    WCHAR* s = GetCommandLine();
    s = removeExecPath(s);
    std::string result;
    {
        HANDLE hRead, hWrite;
        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle = TRUE;
        if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
            std::cerr << "CreatePipe failed." << std::endl;
            return 1;
        }

        STARTUPINFOW si0;
        PROCESS_INFORMATION pi0;
        ZeroMemory(&si0, sizeof(si0));
        ZeroMemory(&pi0, sizeof(pi0));

        si0.cb = sizeof(STARTUPINFO);
        GetStartupInfoW(&si0);
        si0.hStdError = GetStdHandle(STD_ERROR_HANDLE);
        si0.hStdOutput = hWrite;
        si0.dwFlags = STARTF_USESTDHANDLES;


        if (!CreateProcess(NULL, s, NULL, NULL, TRUE, 0, NULL, NULL, &si0, &pi0)) {
            std::cerr << "CreateProcess failed." << std::endl;
            return 1;
        }

        CloseHandle(hWrite);

        DWORD dwRead;
        CHAR chBuf[4096];
        BOOL bSuccess = FALSE;

        for (;;) {
            bSuccess = ReadFile(hRead, chBuf, 4096, &dwRead, NULL);
            if (!bSuccess || dwRead == 0) break;
            result.append(chBuf, dwRead);
        }
        CloseHandle(hRead);
        CloseHandle(pi0.hProcess);
        CloseHandle(pi0.hThread);
    }

    // Convert UTF-8 to UTF-16 (WCHAR)
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &result[0], (int)result.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &result[0], (int)result.size(), &wstrTo[0], size_needed);

    if (wstrTo.empty()) return 1;
    //Run main program
    //MessageBoxA(0, result.c_str(), "", 0);
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
    CreateProcess(NULL, &wstrTo[0], NULL, NULL, TRUE, CREATE_NEW_PROCESS_GROUP, NULL, NULL, &si, &pi);
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

    return 0;
}