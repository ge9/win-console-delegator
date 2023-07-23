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
    if (!CreateProcess(NULL,cmd,NULL,NULL,TRUE,0,NULL,NULL,&siStartInfo,&piProcInfo))
    {
        std::wcerr << L"CreateProcess failed" << std::endl;
        exit(1);
    }
    free(cmd);
    CloseHandle(piProcInfo.hThread);
    return piProcInfo.hProcess;
}

std::vector<std::wstring> SplitCommands(const std::wstring& input) {
    std::vector<std::wstring> commands;
    std::wstring currentCommand;

    for (std::size_t i = 0; i < input.size(); ++i) {
        if (input[i] == L'|') {
            if (i + 1 < input.size() && input[i + 1] == L'|') { // Next character is also a pipe
                currentCommand += L'|'; // Treat it as an escaped pipe
                ++i; // Skip the next character
            }
            else { // It's a separator
                commands.push_back(currentCommand); // Save current command
                currentCommand.clear(); // Prepare for the next one
            }
        }
        else {
            currentCommand += input[i];
        }
    }

    commands.push_back(currentCommand); // the last command

    // Trim leading spaces from commands
    for (auto& command : commands) {
        command = std::regex_replace(command, std::wregex(L"^\\s+"), L"");
    }

    return commands;
}

int main() {
    WCHAR* s = GetCommandLine();
    s = removeExecPath(s);

    std::vector<std::wstring> commands = SplitCommands(std::wstring(s));
    std::vector<HANDLE> processHandles;

    HANDLE prevPipe = GetStdHandle(STD_INPUT_HANDLE);
    for (int i = 0; i < commands.size(); ++i) {
        //std::wcerr << commands[i]+L"\n" << std::endl;

        HANDLE pipeIn = NULL;
        HANDLE pipeOut = GetStdHandle(STD_OUTPUT_HANDLE);

        if (i < commands.size() - 1) {
            HANDLE readPipe, writePipe;
            CreatePipe(&readPipe, &writePipe, NULL, 0);
            SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
            SetHandleInformation(writePipe, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
            pipeIn = readPipe;
            pipeOut = writePipe;
        }

        HANDLE pi = ExecuteCommand(commands[i], prevPipe, pipeOut);
        processHandles.push_back(pi);

        if (i > 0) CloseHandle(prevPipe);
        if (i < commands.size() - 1) CloseHandle(pipeOut);
        prevPipe = pipeIn;
    }
    // Wait for all processes to finish
    WaitForMultipleObjects(processHandles.size(), processHandles.data(), TRUE, INFINITE);

    for (auto handle : processHandles) CloseHandle(handle);

    return 0;
}