#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

int main(int argc, char** argv)
{
    //Creating 3 types of structs. 
    WSADATA wsa;   // will be used to initiate initiate ws2_32.dll by the process
    SOCKET srv_socket, client_socket;   // Will hold the file descriptor of the socket.
    struct sockaddr_in server, client;  // Addresses struct's for the SRV AND after the connections is establised, for the client as well.

    // Initiates usage of ws2_32.dll by the process. This is a MUST for every script that uses <winsock2.h>
    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("\nFailed. Error code: %d", WSAGetLastError()); //If fails: print the last error code in a socket related operation.
        return 1;
    }
    printf("\nInitialised");

    //Create the local socket.
    if ((srv_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        printf("\nCouldn't create the socket. Error code: %d", WSAGetLastError());
        return 1;
    }
    printf("\nSocket.");

    //Create an address struct for the server
    server.sin_family = AF_INET;
    server.sin_port = htons(4444);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(srv_socket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
    {
        printf("\nCouldn't Bind. Error code: %d", WSAGetLastError());
        return 1;
    }
    printf("\nBind done.");

    //listen for incoming connections. Max of 3 in connection queue
    listen(srv_socket, 3);
    printf("\nWaiting for incoming connections....");

    int c = sizeof(struct sockaddr_in);
    client_socket = accept(srv_socket, (struct sockaddr*)&client, &c);
    if (client_socket == INVALID_SOCKET)
    {
        printf("\naccept failed. Error code: %d", WSAGetLastError());
        return 1;
    }
    printf("\nconnection establised.");

    //Print the details of the client address
    printf("[+] Connected to %s \non port %d", inet_ntoa(client.sin_addr), ntohs(client.sin_port));


    //Send a message 
    char message_send[100] = "calc.exe \0";
    char message_recive[100];
    int sent_bytes = send(client_socket, message_send, sizeof(message_send), 0);


    //recive a response.
    int byte_amount = recv(client_socket, message_recive, sizeof(message_recive), MSG_PEEK);
    if (byte_amount == SOCKET_ERROR)
    {
        printf("\nrecv Error code: %d", WSAGetLastError());
        return 1;
    }
    else
    {
        printf("\n[+] Size of message recived: %d", byte_amount);
        printf("\n[+] Message recived: %s", message_recive);
    }


    //Close the socket
    closesocket(srv_socket);

    //Terminate the usage of ws2_32.dll by the process.
    WSACleanup();

    printf("Press key to exit...");
    getchar();


}