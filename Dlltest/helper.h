#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <vector>
#include <string>

DWORD getPid(const char* execName);

bool patch(BYTE* dst, BYTE* src, unsigned int size);

bool nop(BYTE* dst, unsigned int size);

uintptr_t getProcessModuleAddress(const uintptr_t pid);

uintptr_t calculateDynamicMemoryAddress(uintptr_t ptr, std::vector<unsigned int> offsets);

void printError(const char* msg);

class Header
{
};
