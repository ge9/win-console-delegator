#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include "..\_header.h"

HANDLE ExecuteCommand(const std::wstring& command, HANDLE inputPipe, HANDLE outputPipe) {
    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;

    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    siStartInfo.hStdOutput = outputPipe;
    siStartInfo.hStdInput = inputPipe;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    wchar_t* cmd = _wcsdup(command.c_str());
    if (!CreateProcess(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &siStartInfo, &piProcInfo))
    {
        std::wcerr << L"CreateProcess failed" << std::endl;
        exit(1);
    }
    free(cmd);
    CloseHandle(piProcInfo.hThread);
    return piProcInfo.hProcess;
}

std::pair< std::wstring, std::wstring> SplitCommands(const std::wstring& input) {
    std::wstring remaining;
    std::wstring currentCommand;

    for (std::size_t i = 0; i < input.size(); ++i) {
        if (input[i] == L'|') {
            if (i + 1 < input.size() && input[i + 1] == L'|') { // Next character is also a pipe
                currentCommand += L'|'; // Treat it as an escaped pipe
                ++i; // Skip the next character
            }
            else { // It's a separator
                remaining = input.substr(i + 1);
                break;
            }
        }
        else {
            currentCommand += input[i];
        }
    }

    // Trim leading spaces from remainings
    remaining = std::regex_replace(remaining, std::wregex(L"^\\s+"), L"");

    return std::make_pair(currentCommand, remaining);
}

int main() {
    WCHAR* s = GetCommandLine();
    s = removeExecPath(s);

    auto commands = SplitCommands(std::wstring(s));
    HANDLE processHandles[2];

    {
        HANDLE readPipe, writePipe;
        CreatePipe(&readPipe, &writePipe, NULL, 0);
        SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
        SetHandleInformation(writePipe, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
        HANDLE h0 = ExecuteCommand(commands.first, GetStdHandle(STD_INPUT_HANDLE), writePipe);
        processHandles[0] = h0;
        CloseHandle(writePipe);

        HANDLE h1 = ExecuteCommand(commands.second, readPipe, GetStdHandle(STD_OUTPUT_HANDLE));
        processHandles[1] = h1;
        CloseHandle(readPipe);
    }
    // Wait for all processes to finish
    WaitForMultipleObjects(2, processHandles, TRUE, INFINITE);
    CloseHandle(processHandles[0]);
    CloseHandle(processHandles[1]);

    return 0;
}