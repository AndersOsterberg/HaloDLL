#include "pch.h"
#include "helper.h"
#include "windows.h"

DWORD getPid(const char* execName)
{
	DWORD pid = 0;
	HWND hwnd = FindWindowA(NULL, execName);
	if (hwnd == NULL)
	{
		return 0;
	}
	GetWindowThreadProcessId(hwnd, &pid);

	return pid;
}

//Skriv över minne
//TODO om det går fel tidigt sluta direkt
bool patch(BYTE* dst, BYTE* src, unsigned int size)
{
	DWORD oldProtect;
	BOOL virtualSucces1 = VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldProtect);
	std::cout << "Change access success1: " << virtualSucces1 << std::endl;

	//BOOL writeSuccess   = WriteProcessMemory(handle, dst, src, size, nullptr); gamla icke internal
	memcpy(dst, src, size);

	//Återställ gamla rättigheterna
	BOOL virtualSuccess2 = VirtualProtect(dst, size, oldProtect, &oldProtect);
	std::cout << "Change access success2: " << virtualSuccess2 << std::endl;

	return virtualSucces1 && virtualSuccess2;
}

//nop:ar instruktion av storlek size
bool nop(BYTE* dst, unsigned int size)
{
	DWORD oldProtect;
	BOOL virtualSucces1 = VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldProtect);
	std::cout << "Change access success1: " << virtualSucces1 << std::endl;

	//Skriv över med nop * size
	memset(dst, 0x90, size);

	//Återställ gamla rättigheterna
	BOOL virtualSuccess2 = VirtualProtect(dst, size, oldProtect, &oldProtect);
	std::cout << "Change access success2: " << virtualSuccess2 << std::endl;

	return virtualSucces1 && virtualSuccess2;
}

//Till större del tagen från microsoft
uintptr_t getProcessModuleAddress(const uintptr_t pid)
{
	HANDLE hSnap;
	MODULEENTRY32 moduleEntry;
	moduleEntry.dwSize = sizeof(MODULEENTRY32);
	uintptr_t moduleBaseAddress = 0;

	//  Take a snapshot of all modules, processes and heap in the specified process.
	hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid); // | TH32CS_SNAPMODULE32 om programmet är 32 bitars(?)
	if (hSnap == INVALID_HANDLE_VALUE) {
		printError("CreateToolhelp32Snapshot (of modules)");
		return false;
	}

	//Retrieve information about the first module,
	if (!Module32First(hSnap, &moduleEntry)) {
		printError("Module32First");  // Show cause of failure
		CloseHandle(hSnap);     // Must clean up the snapshot object!
		return false;
	}

	//Behöver inte loopa, första modulen är den vi vill ha, andra program kommer man troligen behöva jämföra namnet på modulen med den vi är ute efter
	moduleBaseAddress = (uintptr_t) moduleEntry.modBaseAddr;

	//Clean up
	CloseHandle(hSnap);

	return moduleBaseAddress;
}

uintptr_t calculateDynamicMemoryAddress(uintptr_t ptr, std::vector<unsigned int> offsets)
{
    uintptr_t addr = ptr;
    for (unsigned int i = 0; i < offsets.size(); ++i)
    {
		//BOOL readSuccess = ReadProcessMemory(hProc, (BYTE*)addr, &addr, sizeof(addr), 0);
		//Eftersom den är internal behöver man inte längre ReadProcessMemory
		addr = *(uintptr_t*)addr;
        addr += offsets[i];
    }
    return addr;
}

//Direkt tagen från microsoft
void printError(const char* msg)
{
	DWORD eNum;
	TCHAR sysMsg[256];
	TCHAR *p;

	eNum = GetLastError();
	FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, eNum, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		sysMsg, 256, NULL);

	// Trim the end of the line and terminate it with a null
	p = sysMsg;
	while ((*p > 31) || (*p == 9))
		++p;
	do {
		*p-- = 0;
	} while ((p >= sysMsg) && ((*p == '.') || (*p < 33)));

	// Display the message
	std::cout << "\n  WARNING: %s failed with error %ld (%s)", msg, eNum, sysMsg;
}