#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstdlib>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include "sqlite3.h"

#define SERVER_PORT 8080
#define MAX_PENDING 5
#define MAX_LINE 256

// SQLite database connection
sqlite3* db;
int s;

// Struct to hold user information
struct User {
    char username[20];
    int balance;
};

// Callback function for SQLite3 
static int callback(void* NotUsed, int argc, char** argv, char** azColName) {
    int i;
    for (i = 0; i < argc; i++) {
        std::cout << azColName[i] << " = " << (argv[i] ? argv[i] : "NULL") << std::endl;
    }
    return 0;
}



// Function to open SQLite database
bool openDatabase() {
    int rc = sqlite3_open("stocks.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    return true;
}


// Function to close SQLite database
void closeDatabase() {
    sqlite3_close(db);
}

// Function to check if user exists, and create one if not
void checkOrCreateUser() {
    // Open SQLite database
    if (!openDatabase()) {
        exit(1);
    }

    // Check if user exists
    const char* userSql = "SELECT COUNT(*) FROM Users WHERE user_id = 1;";
    int userCount = 0;
    sqlite3_stmt* userStmt;
    if (sqlite3_prepare_v2(db, userSql, -1, &userStmt, NULL) != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        closeDatabase();
        exit(1);
    }
    if (sqlite3_step(userStmt) == SQLITE_ROW) {
        userCount = sqlite3_column_int(userStmt, 0);
    }
    sqlite3_finalize(userStmt);

    // If user doesn't exist, create one
    if (userCount == 0) {
        const char* insertUserSql = "INSERT INTO Users (user_id, username, balance) VALUES (1, 'John Doe', 100);";
        if (sqlite3_exec(db, insertUserSql, NULL, NULL, NULL) != SQLITE_OK) {
            std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
            closeDatabase();
            exit(1);
        }
    }

    // Close SQLite database connection
    closeDatabase();
}



// BUY command
void handleBuy(int new_s, char* command) {
    char stock_symbol[20];
    float stock_amount;
    float price_per_stock;
    int user_id;

    // Parse command
    if (sscanf(command, "BUY %s %f %f %d", stock_symbol, &stock_amount, &price_per_stock, &user_id) != 4) {
        send(new_s, "Invalid command format", 23, 0);
        return;
    }
}



// handle SELL 
void handleSell(int new_s, char* command) {
    char stock_symbol[20];
    float stock_amount;
    float price_per_stock;
    int user_id;

    // Parse command
    if (sscanf(command, "SELL %s %f %f %d", stock_symbol, &stock_amount, &price_per_stock, &user_id) != 4) {
        send(new_s, "Invalid command format", 23, 0);
        return;
    }

    // Open SQLite database
    if (!openDatabase()) {
        send(new_s, "Database error", 14, 0);
        return;
    }

    // Implement logic to update user's balance and Stocks table

    float new_balance = 100.0 + (stock_amount * price_per_stock);
    char response[MAX_LINE];
    sprintf(response, "200 OK\nSOLD: New balance: %.1f %s. USD $%.2f",
        stock_amount, stock_symbol, new_balance);
    send(new_s, response, strlen(response), 0);

    // Close SQLite db
    closeDatabase();
}


// handle LIST 
void handleList(int new_s) {

    // Open SQLite database
    int rc = sqlite3_open("stocks.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        send(new_s, "Database error", 14, 0);
        return;
    }

    // Query table and send results back to client
    const char* sql = "SELECT * FROM Stocks;";
    char response[MAX_LINE] = "200 OK\n";
    rc = sqlite3_exec(db, sql, callback, response, NULL);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        send(new_s, "Database error", 14, 0);
        sqlite3_close(db);
        return;
    }

    // Send response to client
    send(new_s, response, strlen(response), 0);

    // Close SQLite database connection
    sqlite3_close(db);
}

// Function to handle BALANCE command
void handleBalance(int new_s) {
    // Open SQLite database
    int rc = sqlite3_open("stocks.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        send(new_s, "Database error", 14, 0);
        return;
    }

    // Query USD balance for user 1 and send result back to client
    const char* sql = "SELECT balance FROM Users WHERE user_id = 1;";
    char response[MAX_LINE] = "200 OK\n";
    rc = sqlite3_exec(db, sql, callback, response, NULL);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        send(new_s, "Database error", 14, 0);
        sqlite3_close(db);
        return;
    }

    // Send response to client
    send(new_s, response, strlen(response), 0);

    // Close SQLite database connection
    sqlite3_close(db);
}

// Function to handle SHUTDOWN command
void handleShutdown(int new_s, int s) {
    send(new_s, "200 OK\n", 7, 0);
    close(new_s);
    close(s);
    sqlite3_close(db);
    exit(0);
}

// Function to handle QUIT command
void handleQuit(int new_s) {
    send(new_s, "200 OK\n", 7, 0);
    close(new_s);
}

// Function to handle client commands
void handleCommand(int new_s, char* command) {
    if (strncmp(command, "BUY", 3) == 0) {
        handleBuy(new_s, command);
    }
    else if (strncmp(command, "SELL", 4) == 0) {
        handleSell(new_s, command);
    }
    else if (strncmp(command, "LIST", 4) == 0) {
        handleList(new_s);
    }
    else if (strncmp(command, "BALANCE", 7) == 0) {
        handleBalance(new_s);
    }
    else if (strncmp(command, "SHUTDOWN", 8) == 0) {
        handleShutdown(new_s, s);
    }
    else if (strncmp(command, "QUIT", 4) == 0) {
        handleQuit(new_s);
    }
    else {
        send(new_s, "Invalid command", 16, 0);
    }
}


int main() {
    struct sockaddr_in sin;
    char buf[MAX_LINE];
    int buf_len, addr_len;
    int new_s;

    //users part
    checkOrCreateUser();

    // Build address data structure
    bzero((char*)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(SERVER_PORT);

    // Setup passive open
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("simplex-talk: socket");
        exit(1);
    }
    if ((bind(s, (struct sockaddr*)&sin, sizeof(sin))) < 0) {
        perror("simplex-talk: bind");
        exit(1);
    }
    listen(s, MAX_PENDING);

    // Accept incoming connections and handle client commands
    while (1) {
        if ((new_s = accept(s, (struct sockaddr*)&sin, (socklen_t*)&addr_len)) < 0) {
            perror("simplex-talk: accept");
            exit(1);
        }

        // Receive command from client
        while ((buf_len = recv(new_s, buf, sizeof(buf), 0)) > 0) {
            handleCommand(new_s, buf);
        }
        close(new_s);
    }

    return 0;
}