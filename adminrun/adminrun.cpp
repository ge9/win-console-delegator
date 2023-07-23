#include <windows.h>

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPrevInst,
    LPWSTR lpCmdLine, int intShowCmd)
{
    WCHAR* s = lpCmdLine;
    if (*s == '"') {
        ++s;
        while (*s)
            if (*s++ == '"') break;
    }
    else {
        while (*s && *s != ' ' && *s != '\t') ++s;
    }
    if (*s != L'\0') {//with argument
        *s++ = L'\0';//separate command line at the first space
        while (*s == ' ' || *s == '\t') s++;
    }

    SHELLEXECUTEINFO shExInfo = { 0 };
    shExInfo.cbSize = sizeof(shExInfo);
    shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    shExInfo.hwnd = 0;
    shExInfo.lpVerb = L"runas";
    shExInfo.lpFile = lpCmdLine;
    shExInfo.lpParameters = s;
    shExInfo.lpDirectory = 0;
    shExInfo.nShow = SW_SHOW;
    shExInfo.hInstApp = 0;
    if (ShellExecuteEx(&shExInfo))
    {
        WaitForSingleObject(shExInfo.hProcess, INFINITE);  // 実行するプロセスが終了するのを待つ
        CloseHandle(shExInfo.hProcess);  // プロセスのハンドルを閉じる
    }

    return 0;
}