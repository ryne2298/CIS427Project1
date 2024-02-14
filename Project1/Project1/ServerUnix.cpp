#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sqlite3.h>

#define SERVER_PORT 8080
#define MAX_PENDING  5
#define MAX_LINE     256



//adding users part
struct User {
    char username[20];
    int balance;
};

// Function to check if user exists, and create one if not
void checkOrCreateUser(struct User* users, int* numUsers) {
    if (*numUsers == 0) {
        strcpy(users[0].username, "user1");
        users[0].balance = 100;
        (*numUsers)++;
        printf("Created new user: %s with balance: %d\n", users[0].username, users[0].balance);
    }
}



int main()
{
    struct sockaddr_in sin;
    char buf[MAX_LINE];
    int buf_len, addr_len;
    int s, new_s;
    //adding users part
    struct User users[10]; // Array to hold user information
    int numUsers = 0;


    //build address data structure 
    bzero((char*)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(SERVER_PORT);

    // setup passive open 
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("simplex-talk: socket");
        exit(1);
    }
    if ((bind(s, (struct sockaddr*)&sin, sizeof(sin))) < 0) {
        perror("simplex-talk: bind");
        exit(1);
    }
    listen(s, MAX_PENDING);

    // wait for connection, then receive and print text
    while (1) {
        if ((new_s = accept(s, (struct sockaddr*)&sin, &addr_len)) < 0) {
            perror("simplex-talk: accept");
            exit(1);
        }
        while (buf_len = recv(new_s, buf, sizeof(buf), 0))
            fputs(buf, stdout);
        close(new_s);
    }


    // Check if there is at least one user, create one if not
    checkOrCreateUser(users, &numUsers);

    while (buf_len = recv(new_s, buf, sizeof(buf), 0)) {
        printf("Received: %s", buf);
        // Here is echo back the message with user's balance but it can be changed
        char response[MAX_LINE];
        sprintf(response, "You said: %s\nYour balance is: %d\n", buf, users[0].balance);
        send(new_s, response, strlen(response), 0);
    }
    close(new_s);
}
close(s);
return 0;
}

}
