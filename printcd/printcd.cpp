#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>
#include <fcntl.h>
#include <io.h>

int main() {
    wchar_t buffer[MAX_PATH];
    // Get current directory as wide-char string
    DWORD dwResult = GetCurrentDirectory(MAX_PATH, buffer);
    // Convert wide-char string to multibyte string
    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, nullptr, 0, nullptr, nullptr);
    std::vector<char> mbPath(bufferSize);
    int result = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, &mbPath[0], bufferSize, nullptr, nullptr);
    std::cout << &mbPath[0] << std::endl;
    return 0;
}
