#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>


#define SERVER_PORT "8080" 
#define MAX_LINE 256

// handle BUY 
void handleBuy(SOCKET clientSocket, char* command) {
     send(clientSocket, command, strlen(command), 0);

    // response
    char response[MAX_LINE];
    int bytesReceived = recv(clientSocket, response, MAX_LINE, 0);
    if (bytesReceived > 0) {
        response[bytesReceived] = '\0';
        std::cout << "Received response from server: " << response << std::endl;
    }
    else {
        std::cerr << "Error receiving response from server" << std::endl;
    }
}

// handle SELL 
void handleSell(SOCKET clientSocket, char* command) {
     send(clientSocket, command, strlen(command), 0);

    
    char response[MAX_LINE];
    int bytesReceived = recv(clientSocket, response, MAX_LINE, 0);
    if (bytesReceived > 0) {
        response[bytesReceived] = '\0';
        std::cout << "Received response from server: " << response << std::endl;
    }
    else {
        std::cerr << "Error receiving response from server" << std::endl;
    }
}

// handle LIST 
void handleList(SOCKET clientSocket) {
     send(clientSocket, "LIST\n", 6, 0);

   
    char response[MAX_LINE];
    int bytesReceived = recv(clientSocket, response, MAX_LINE, 0);
    if (bytesReceived > 0) {
        response[bytesReceived] = '\0';
        std::cout << "Received response from server: " << response << std::endl;
    }
    else {
        std::cerr << "Error receiving response from server" << std::endl;
    }
}

// handle BALANCE
void handleBalance(SOCKET clientSocket) {
     send(clientSocket, "BALANCE\n", 8, 0);

   
    char response[MAX_LINE];
    int bytesReceived = recv(clientSocket, response, MAX_LINE, 0);
    if (bytesReceived > 0) {
        response[bytesReceived] = '\0';
        std::cout << "Received response from server: " << response << std::endl;
    }
    else {
        std::cerr << "Error receiving response from server" << std::endl;
    }
}

// handle SHUTDOWN 
void handleShutdown(SOCKET clientSocket) {
     send(clientSocket, "SHUTDOWN\n", 9, 0);

    
    char response[MAX_LINE];
    int bytesReceived = recv(clientSocket, response, MAX_LINE, 0);
    if (bytesReceived > 0) {
        response[bytesReceived] = '\0';
        std::cout << "Received response from server: " << response << std::endl;
    }
    else {
        std::cerr << "Error receiving response from server" << std::endl;
    }

    // Close socket and clean up
    closesocket(clientSocket);
    WSACleanup();
    exit(0);
}

// handle QUIT 
void handleQuit(SOCKET clientSocket) {
     send(clientSocket, "QUIT\n", 5, 0);

   
    char response[MAX_LINE];
    int bytesReceived = recv(clientSocket, response, MAX_LINE, 0);
    if (bytesReceived > 0) {
        response[bytesReceived] = '\0';
        std::cout << "Received response from server: " << response << std::endl;
    }
    else {
        std::cerr << "Error receiving response from server" << std::endl;
    }

    // Close 
    closesocket(clientSocket);
}





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

    // Command loop
    while (true) {
        std::cout << "Enter command (BUY, SELL, LIST, BALANCE, SHUTDOWN, QUIT): ";
        std::cin.getline(buf, sizeof(buf));
        len = (int)strlen(buf);

        if (strncmp(buf, "BUY", 3) == 0) {
            handleBuy(s, buf);
        }
        else if (strncmp(buf, "SELL", 4) == 0) {
            handleSell(s, buf);
        }
        else if (strncmp(buf, "LIST", 4) == 0) {
            handleList(s);
        }
        else if (strcmp(buf, "BALANCE") == 0) {
            handleBalance(s);
        }
        else if (strcmp(buf, "SHUTDOWN") == 0) {
            handleShutdown(s);
        }
        else if (strcmp(buf, "QUIT") == 0) {
            handleQuit(s);
        }
        else {
            //to the server
             send(s, buf, len, 0);

            //responce
            int bytesReceived = recv(s, buf, MAX_LINE, 0);
            if (bytesReceived > 0) {
                buf[bytesReceived] = '\0';
                std::cout << "Received response from server: " << buf << std::endl;
            }
            else {
                std::cerr << "Error receiving response from server" << std::endl;
            }
        }
    }

    //exceptions to handle error if user does not exist
    while (fgets(buf, sizeof(buf), stdin)) {
        buf[MAX_LINE - 1] = '\0';
        len = (int)strlen(buf) + 1;
        send(s, buf, len, 0);

        // response
        int bytes_received = recv(s, buf, sizeof(buf), 0);
        if (bytes_received > 0) {
            buf[bytes_received] = '\0';
            std::cout << "Received response from server: " << buf << std::endl;

            
            if (strstr(buf, "200 OK") != nullptr) {
                


            }
            else {
                 std::cerr << "Connection closed by server " << std::endl;
                
            }
        }
        else {
            std::cerr << "Error receiving response from server:   " << WSAGetLastError() << std::endl;
            
        }
    }

    shutdown(s, SD_SEND);
    closesocket(s);
    WSACleanup();
    return 0;
}
