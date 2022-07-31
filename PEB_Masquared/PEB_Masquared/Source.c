#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <winternl.h>


// Printing System error's
void Error(const char* msg)
{
	printf("\n### ERROR ###  %s  (%d)", msg, GetLastError());
	exit(EXIT_FAILURE);
}



int main()
{

// Getting the offeset of the pointer to the PEB from the start of the TEB
#ifdef _M_IX86 
	PPEB ProcEnvBlk = (PPEB)__readfsdword(0x30);
#else
	PPEB ProcEnvBlk = (PPEB)__readgsqword(0x60);
#endif


	// Change the current directory of the process
	int result = SetCurrentDirectoryW(L"C:\\Users\\ariels\\");
	if (result == 0)
		Error("SetCurrentDirectory failed");

	// Change the Process image path and command line
	WCHAR path[] = L"c:\\windows\\system32\\notepad.exe\0";
	memcpy(ProcEnvBlk->ProcessParameters->ImagePathName.Buffer, path, ProcEnvBlk->ProcessParameters->ImagePathName.Length);
	memcpy(ProcEnvBlk->ProcessParameters->CommandLine.Buffer, path, ProcEnvBlk->ProcessParameters->CommandLine.Length);


	STARTUPINFO si = {0};
	PROCESS_INFORMATION pi = { 0 };
	result = CreateProcessA(NULL, "c:\\windows\\system32\\cmd.exe", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	if (result == 0)
		Error("CreateProcessA failed");


	getchar();



}