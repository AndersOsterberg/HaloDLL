// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <vector>
#include <thread>
#include <chrono>
#include <vector>
#include "helper.h"


//typedef the function prototype
typedef void(__fastcall* _Write_Function)(short param1, int param2, short param3, char param4);

DWORD WINAPI HackThread(HMODULE hModule)
{
	//Create an instance of the function and assign it to an address
	int offsetToFunction = 0xafb640;
	int offsetToGraphicObjekt = 0x2bf44e0;
	
	int offsetToText = 0xE4AE * 2;
	//Ers�tter 'Picked up the flag'
	wchar_t	textToWrite[19]	 = L"Hej fr�n Anders   ";

	AllocConsole();
	FILE* f;
	//�ppna file stream, conout �r out som std out, "w" write
	freopen_s(&f, "CONOUT$", "w", stdout);

	std::cout << "Hej konsol" << std::endl;

	uintptr_t moduleBaseAddress = (uintptr_t)GetModuleHandle(L"halo1.dll");
	std::cout << std::hex << "moduleBaseAddress: " << moduleBaseAddress << std::endl;

	uintptr_t functionAddress   = moduleBaseAddress + offsetToFunction;
	std::cout << "functionAddress: " << functionAddress << std::endl;

	uintptr_t graphicObjectPointer = moduleBaseAddress + offsetToGraphicObjekt;
	
	uintptr_t graphicObjectAddress = *(uintptr_t*)graphicObjectPointer;
	graphicObjectAddress += 0x18;
	std::cout << "graphicObjetAddress: " << graphicObjectAddress << std::endl;

	uintptr_t enormousStringPointer = *(uintptr_t*)graphicObjectAddress;
	std::cout << "enormousStringPointer: " << enormousStringPointer << std::endl;

	uintptr_t stringLocationToOverwrite = enormousStringPointer + offsetToText;
	std::cout << "toWrite: " << stringLocationToOverwrite << std::endl;
	wchar_t* stringLocation = (wchar_t*)stringLocationToOverwrite;
	wcscpy(stringLocation, textToWrite);
	
	_Write_Function writeFunction = (_Write_Function)(moduleBaseAddress+offsetToFunction);

	//short param1 = 0; Originalv�rde
	short param1 = 0;
	//int   param2 = 0xED350BC0; Originalv�rde, active camo: 0xE93E07CA
	//Notera ENDAST de 4 sista bytes anv�nds
	int   notReallyOffsetButKindOf = 0x0818; //ger offset E4AE*2
	//short param3 = 1; Originalv�rde
	//Kanske vilken typ det �r, �r den 0 s� uppdaterar den inte hur m�nga plasmagranater man har tagit upp
	short CanItBeMoreThanOne = 0;
	//char  param4 = 0xFF; Originalv�rde
	char  offsetToOffset = 0x0;

	while (true)
	{

		//skriv
		if (GetAsyncKeyState(VK_CAPITAL) & 1) //caps lock
		{
			std::cout << "Hejsan med caps" << std::endl;
			
			writeFunction(param1, notReallyOffsetButKindOf, CanItBeMoreThanOne, offsetToOffset);
			//std::cout << std::hex << "param4: "  << (short)notReallyOffsetButKindOf <<  std::endl;
			//notReallyOffsetButKindOf = notReallyOffsetButKindOf + 1;
			

			
			
		}
		//Bryt dll:n
		if (GetAsyncKeyState(VK_RETURN) & 1) //VK_RETURN �r enter
		{
			std::cout << "Break" << std::endl;
			break;
		}
		
		Sleep(5);
	}

	//Att g�ra allt det h�r till�ter en att injecta om sen om man vill
	fclose(f);
	FreeConsole();
	FreeLibraryAndExitThread(hModule, 0);
	return 0;

}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    //Load library blir kallad
    case DLL_PROCESS_ATTACH:
		//Skapar en tr�d f�r programmet som kallar p� en
		CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HackThread, hModule, 0, nullptr));
    case DLL_THREAD_ATTACH:

    case DLL_THREAD_DETACH:

    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
