#include "FunctionsSignatures.h"
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <winternl.h>
#include <shlwapi.h>
#include <string.h>

#pragma comment(lib, "Shlwapi.lib")





int CompareUnicodeStrings(WCHAR substring[], WCHAR bigstring[])
{

	_wcslwr_s(substring, MAX_PATH);
	_wcslwr_s(bigstring, MAX_PATH);

	int result = 0;
	if (StrStrW(bigstring, substring) != NULL)
	{
		result = 1;
	}

	return result;
}


HMODULE WINAPI pGetModuleHandle(LPCWSTR sModuleName) 
{

	// Getting the offeset of PPEB from the start of the TEB
#ifdef _M_IX86 
	PEB* ProcEnvBlk = (PEB*)__readfsdword(0x30);
#else
	PEB* ProcEnvBlk = (PEB*)__readgsqword(0x60);
#endif

	// Finding the address of the Head node of the linked list, which represents all the models that are loaded into the process.
	PEB_LDR_DATA* Ldr = ProcEnvBlk->Ldr;
	LIST_ENTRY* ModuleList = &Ldr->InMemoryOrderModuleList; 

	// Iterating to the next node. This will be our starting point.
	LIST_ENTRY* pStartListEntry = ModuleList->Flink;

	// Iterating through the linked list.
	WCHAR big_string[MAX_PATH] = { 0 };
	WCHAR sub_string[MAX_PATH] = { 0 };
	for (LIST_ENTRY* pListEntry = pStartListEntry; pListEntry != ModuleList; pListEntry = pListEntry->Flink) {

		// Getting the address of current LDR_DATA_TABLE_ENTRY (whic represents the DLL).
		LDR_DATA_TABLE_ENTRY* pEntry = (LDR_DATA_TABLE_ENTRY*)((BYTE*)pListEntry - sizeof(LIST_ENTRY));

		// Checking if this is the DLL we are looking for
		memset(big_string, 0, MAX_PATH * sizeof(WCHAR));
		memset(sub_string, 0, MAX_PATH * sizeof(WCHAR));
		wcscpy_s(big_string, MAX_PATH, pEntry->FullDllName.Buffer);
		wcscpy_s(sub_string, MAX_PATH, sModuleName);
		if (CompareUnicodeStrings(sub_string, big_string))
		{
			// Returning the DLL base address.
			return (HMODULE)pEntry->DllBase;
		}
	}

	// The needed DLL wasn't found
	printf("\nHMODULE wasn't found!");
	return NULL;
}



FARPROC WINAPI pGetProcAddress(HMODULE hMod, char* sProcName) 
{
	char* pBaseAddr = (char*)hMod;

	// Creatring a pointer to an IMAGE_DOS_HEADER struct. This struct represents the first 64 bytes of a file.
	IMAGE_DOS_HEADER* pDosHdr = (IMAGE_DOS_HEADER*)pBaseAddr;
 
	IMAGE_NT_HEADERS* pNTHdr = (IMAGE_NT_HEADERS*)(pBaseAddr + pDosHdr->e_lfanew);

	// IMAGE_NT_HEADERS contains the IMAGE_OPTIONAL_HEADER inside of him.
	IMAGE_OPTIONAL_HEADER* pOptionalHdr = &pNTHdr->OptionalHeader;

	// Get a pointer to a data directory struct that contains the RVA of the export directory struct.
	IMAGE_DATA_DIRECTORY* pExportDataDir = (IMAGE_DATA_DIRECTORY*)(&pOptionalHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]);

	// The data directory we recived above contains the RVA (RVA from the BaseAddress) of the IMAGE_EXPORT_DIRECTORY struct.
	IMAGE_EXPORT_DIRECTORY* pExportDirAddr = (IMAGE_EXPORT_DIRECTORY*)(pBaseAddr + pExportDataDir->VirtualAddress);

	// Get the addresses of the 3 Export directory arrays. 
	DWORD* pEAT = (DWORD*)(pBaseAddr + pExportDirAddr->AddressOfFunctions);
	DWORD* pFuncNameTbl = (DWORD*)(pBaseAddr + pExportDirAddr->AddressOfNames);
	WORD* pHintsTbl = (WORD*)(pBaseAddr + pExportDirAddr->AddressOfNameOrdinals);


	// =====================================   Find the needed function in the Export Address table using it's name    ========================================
	
	void* pProcAddr = NULL;
	// Iterate through the Functions Names array 
	for (DWORD i = 0; i < pExportDirAddr->NumberOfNames; i++) {

		char* sTmpFuncName = (char*)pBaseAddr + (DWORD_PTR)pFuncNameTbl[i];

		if (strcmp(sProcName, sTmpFuncName) == 0) {
			
			// Get the address of the needed function. The "calculation" is expalaind in the article
			pProcAddr = (FARPROC)(pBaseAddr + (DWORD_PTR)pEAT[pHintsTbl[i]]);
			break;
		}
	}

	// Check if pProcAddr is withing the export directory address space. If not it will point to a strign that looks like: {DLL Name}.{Function name}
	//This means that the function is forwarded to a different DLL
	if ((char*)pProcAddr >= (char*)pExportDirAddr && (char*)pProcAddr < (char*)(pExportDirAddr + pExportDataDir->Size)) 
	{
		char* sFwdDLL = _strdup((char*)pProcAddr);
		if (!sFwdDLL) return NULL;

		// Parse the {DLL Name}.{Function name}, when the '.' is the delimiter. 
		char* sFwdFunction = strchr(sFwdDLL, '.');
		*sFwdFunction = 0;
		sFwdFunction++;

		// Create a CallBack to LoadLibraryA, so we can later use it to load the external DLL into our process
		HIDDEN_DLL_FUNC_LoadLibrary pLoadLibraryA = NULL;
		pLoadLibraryA = (HIDDEN_DLL_FUNC_LoadLibrary)pGetProcAddress(pGetModuleHandle(L"KERNEL32.DLL"), (char*)"LoadLibraryA");

		// load the DLL
		HMODULE hFwd = pLoadLibraryA(sFwdDLL);
		free(sFwdDLL);						
		if (!hFwd)
			return NULL;

		//recursivly calling our implementation of GetProcAddress, passing the new HMODULE and function name. 
		pProcAddr = pGetProcAddress(hFwd, sFwdFunction);
	}

	return (FARPROC)pProcAddr;
}
