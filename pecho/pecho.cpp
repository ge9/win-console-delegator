#include <windows.h>
#include <iostream>
#include <vector>
#include "..\_header.h"

int main()
{
	WCHAR* s = GetCommandLine();
	s = removeExecPath(s);
	int bufferSize = WideCharToMultiByte(CP_UTF8, 0, s, -1, nullptr, 0, nullptr, nullptr);
	std::vector<char> mbPath(bufferSize);
	int result = WideCharToMultiByte(CP_UTF8, 0, s, -1, &mbPath[0], bufferSize, nullptr, nullptr);
	std::cout << &mbPath[0] << std::endl;
}
