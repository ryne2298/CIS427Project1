#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sqlite3.h>

#define SERVER_PORT 8080
#define MAX_LINE 256

int main(int argc, char* argv[])
{
    //not sure whats this is doing:
    //FILE* fp;
    struct hostent* hp;
    struct sockaddr_in sin;
    char* host;
    char buf[MAX_LINE];
    int s;
    //int len;

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

    
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("simplex-talk: socket");
        exit(1);
    }
    if (connect(s, (struct sockaddr*)&sin, sizeof(sin)) < 0)
    {
        perror("simplex-talk: connect");
        close(s);
        exit(1);
    }
    
    // 
    // Commands
    char command[MAX_LINE];
    strcpy(command, "BUY MSFT 3.4 1.35 1\n");
    strcpy(command, "SELL APPL 2 1.45 1\n");
    strcpy(command, "LIST\n");
    strcpy(command, "BALANCE\n");
    strcpy(command, "SHUTDOWN\n");
    strcpy(command, "QUIT\n");

    // Send command to server
    send(s, command, strlen(command), 0);
    std::cout << "Sent command to server: " << command << std::endl;

    // Receive response from server
    int bytes_received = recv(s, buf, sizeof(buf), 0);
    if (bytes_received > 0) {
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
    else {
        std::cerr << "Error receiving response from server" << std::endl;
    }
}
    close(s);
    return 0;
}
