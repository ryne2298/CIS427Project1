#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>

using namespace std;

#define SERVER_PORT "8080" 
#define MAX_LINE 1024

void initializeWinsock();
SOCKET createConnectSocket(const char* serverAddr, const char* serverPort);
void sendCommand(SOCKET& clientSocket, const std::string& command);
void listenServerMessages(SOCKET& clientSocket);
void cleanup(SOCKET clientSocket);


void sendLoginCommand(SOCKET& clientSocket, const std::string& userId, const std::string& password) {
    std::string loginCommand = "LOGIN " + userId + " " + password + "\n";
    sendCommand(clientSocket, loginCommand);
}

//initializing multithreading




void initializeWinsock() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        exit(1);
    }
}

SOCKET createConnectSocket(const char* serverAddr, const char* serverPort) {
    struct addrinfo hints, * res;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int result = getaddrinfo(serverAddr, serverPort, &hints, &res);
    if (result != 0) {
        WSACleanup();
        std::cerr << "getaddrinfo failed: " << result << std::endl;
        exit(1);
    }

    SOCKET connectSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (connectSocket == INVALID_SOCKET) {
        freeaddrinfo(res);
        WSACleanup();
        std::cerr << "Error at socket(): " << WSAGetLastError() << std::endl;
        exit(1);
    }

    result = connect(connectSocket, res->ai_addr, (int)res->ai_addrlen);
    if (result == SOCKET_ERROR) {
        closesocket(connectSocket);
        connectSocket = INVALID_SOCKET;
    }

    freeaddrinfo(res);

    if (connectSocket == INVALID_SOCKET) {
        WSACleanup();
        std::cerr << "Unable to connect to server!" << std::endl;
        exit(1);
    }

    return connectSocket;
}

void sendCommand(SOCKET& clientSocket, const std::string& command) {
    int sendResult = send(clientSocket, command.c_str(), static_cast<int>(command.length()), 0);
    if (sendResult == SOCKET_ERROR) {
        std::cerr << "send failed: " << WSAGetLastError() << std::endl;
        cleanup(clientSocket);
        exit(1);
    }

    char recvbuf[MAX_LINE];
    int recvResult = recv(clientSocket, recvbuf, MAX_LINE, 0);
    if (recvResult > 0) {
        recvbuf[recvResult] = '\0'; // Null-terminate the buffer
        std::cout << "Server response: " << recvbuf << std::endl;
    }
    else if (recvResult == 0) {
        std::cout << "Connection closed\n";
    }
    else {
        std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
    }
}
void listenServerMessages(SOCKET& clientSocket) {
    char recvbuf[MAX_LINE];
    int recvResult;

    while (true) {
        recvResult = recv(clientSocket, recvbuf, MAX_LINE, 0);
        if (recvResult > 0) {
            recvbuf[recvResult] = '\0'; // Null-terminate the buffer
            std::cout << "Server response: " << recvbuf << std::endl;
        }
        else if (recvResult == 0) {
            std::cout << "Connection closed\n";
            break;
        }
        else {
            std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
            break;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <Server Address> <Server Port>\n";
        return 1;
    }

    const char* serverAddr = argv[1];
    const char* serverPort = argv[2];

    initializeWinsock();
    SOCKET clientSocket = createConnectSocket(serverAddr, serverPort);

    std::thread listenerThread(listenServerMessages, std::ref(clientSocket));

    std::string userId, password;
    std::cout << "Enter UserID: ";
    std::getline(std::cin, userId);
    std::cout << "Enter Password: ";
    std::getline(std::cin, password);

    sendLoginCommand(clientSocket, userId, password);

    std::string command;
    while (true) {
        std::cout << "Enter command (BUY, SELL, DEPOSIT, LIST, WHO, LOOKUP,  BALANCE, SHUTDOWN, QUIT): ";
        std::getline(std::cin, command);

        if (command == "QUIT") {
            sendCommand(clientSocket, "QUIT\n"); // Make sure to notify the server
            break;
        }
        else if (command == "LOGOUT") {
            sendCommand(clientSocket, "LOGOUT\n");
            
        }
        else {
            sendCommand(clientSocket, command);
        }
    }
    if (listenerThread.joinable()) {
        listenerThread.join();
    }

    cleanup(clientSocket);
    return 0;
}


void cleanup(SOCKET clientSocket) {
    int result = shutdown(clientSocket, SD_SEND);
    if (result == SOCKET_ERROR) {
        std::cerr << "shutdown failed: " << WSAGetLastError() << std::endl;
    }
    closesocket(clientSocket);
    WSACleanup();
}

