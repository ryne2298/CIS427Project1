#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib,"ws2_32.lib") 

#define SERVER_PORT  5432
#define MAX_PENDING  5
#define MAX_LINE     256

int main() {
    WSADATA wsaData;
    int iResult;
    SOCKET listenSocket = INVALID_SOCKET, clientSocket = INVALID_SOCKET;
    struct sockaddr_in sin;
    char buf[MAX_LINE];
    int buf_len;
    int addr_len;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        printf("Error at WSAStartup()\n");
        return 1;
    }

    /* build address data structure */
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(SERVER_PORT);

    /* setup passive open */
    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    if (bind(listenSocket, (struct sockaddr*)&sin, sizeof(sin)) == SOCKET_ERROR) {
        printf("Bind failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    if (listen(listenSocket, MAX_PENDING) == SOCKET_ERROR) {
        printf("Listen failed with error: %ld\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    
    while (1) {
        addr_len = sizeof(sin);
        clientSocket = accept(listenSocket, (struct sockaddr*)&sin, &addr_len);
        if (clientSocket == INVALID_SOCKET) {
            printf("Accept failed with error: %d\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
            return 1;
        }

        
        do {
            buf_len = recv(clientSocket, buf, MAX_LINE, 0);
            if (buf_len > 0) {
                printf("Bytes received: %d\n", buf_len);
                // Echo 
                int iSendResult = send(clientSocket, buf, buf_len, 0);
                if (iSendResult == SOCKET_ERROR) {
                    printf("Send failed with error: %d\n", WSAGetLastError());
                    closesocket(clientSocket);
                    WSACleanup();
                    return 1;
                }
            }
            else if (buf_len == 0)
                printf("Connection closing...\n");
            else {
                printf("Recv failed with error: %d\n", WSAGetLastError());
                closesocket(clientSocket);
                WSACleanup();
                return 1;
            }

        } while (buf_len > 0);

        closesocket(clientSocket);
    }

    
    closesocket(listenSocket);
    WSACleanup();

    return 0;
}
