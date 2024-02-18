#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstdio>
#include <string.h>
#include <sqlite3.h>

#define SERVER_PORT 8080
#define MAX_LINE 256

int main(int argc, char* argv[])
{
    nt main(int argc, char* argv[])
    {
        struct hostent* hp;
        struct sockaddr_in sin;
        char* host;
        char buf[MAX_LINE];
        int s;

        if (argc != 2) {
            fprintf(stderr, "usage: %s host\n", argv[0]);
            exit(1);
        }

        host = argv[1];



        // translate host name into IP address 
        hp = gethostbyname(host);
        if (!hp) {
            fprintf(stderr, "simplex-talk: unknown host: %s\n", host);
            exit(1);
        }

        // build address data structure 
        bzero((char*)&sin, sizeof(sin));
        sin.sin_family = AF_INET;
        bcopy(hp->h_addr, (char*)&sin.sin_addr, hp->h_length);
        sin.sin_port = htons(SERVER_PORT);

        // Create socket
        if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
            perror("simplex-talk: socket");
            exit(1);
        }

        // Connect to server
        if (connect(s, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
            perror("simplex-talk: connect");
            close(s);
            exit(1);
        }



        // Commands
        char command[MAX_LINE];

        // Send multiple commands to server
        strcpy(command, "BUY MSFT 3.4 1.35 1\n");
        send(s, command, strlen(command), 0);
        std::cout << "Sent command to server: " << command << std::endl;

        strcpy(command, "SELL APPL 2 1.45 1\n");
        send(s, command, strlen(command), 0);
        std::cout << "Sent command to server: " << command << std::endl;

        strcpy(command, "LIST\n");
        send(s, command, strlen(command), 0);
        std::cout << "Sent command to server: " << command << std::endl;

        strcpy(command, "BALANCE\n");
        send(s, command, strlen(command), 0);
        std::cout << "Sent command to server: " << command << std::endl;

        strcpy(command, "SHUTDOWN\n");
        send(s, command, strlen(command), 0);
        std::cout << "Sent command to server: " << command << std::endl;

        strcpy(command, "QUIT\n");
        send(s, command, strlen(command), 0);
        std::cout << "Sent command to server: " << command << std::endl;

        // Receive response 
        int bytes_received;
        while ((bytes_received = recv(s, buf, sizeof(buf), 0)) > 0) {
            buf[bytes_received] = '\0';
            // Check for "200 OK" response
            if (strncmp(buf, "200 OK", 6) == 0) {
                // Print actual response message
                std::cout << "Received response from server: " << buf + 7 << std::endl;
            }
            else {
                std::cerr << "Error response from server: " << buf << std::endl;
            }
        }

        // Close socket
        close(s);
        return 0;
    }