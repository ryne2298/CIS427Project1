#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT "8080" 
#define MAX_LINE 256

int main(int argc, char* argv[]) {
    WSADATA wsaData;
    struct addrinfo hints, * res, * ptr;
    char* host;
    char buf[MAX_LINE];
    SOCKET s = INVALID_SOCKET;
    int len;

    if (argc == 2) {
        host = argv[1];
    }
    else {
        fprintf(stderr, "usage: simplex-talk host\n");
        exit(1);
    }

    // Initialize Winsock
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", iResult);
        exit(1);
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;


    iResult = getaddrinfo(host, SERVER_PORT, &hints, &res);
    if (iResult != 0) {
        fprintf(stderr, "getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        exit(1);
    }


    for (ptr = res; ptr != NULL; ptr = ptr->ai_next) {
        // Create a SOCKET 
        s = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (s == INVALID_SOCKET) {
            fprintf(stderr, "socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            exit(1);
        }


        iResult = connect(s, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(s);
            s = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(res); 

        if (s == INVALID_SOCKET) {
            fprintf(stderr, "Unable to connect to server!\n");
            WSACleanup();
            exit(1);
        }


    while (fgets(buf, sizeof(buf), stdin)) {
        buf[MAX_LINE - 1] = '\0';
        len = (int)strlen(buf) + 1;
        send(s, buf, len, 0);
    }


    shutdown(s, SD_SEND);


    closesocket(s);
    WSACleanup();
    return 0;
}
