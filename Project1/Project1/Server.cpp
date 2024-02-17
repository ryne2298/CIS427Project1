#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>


#define SERVER_PORT  8080
#define MAX_PENDING  5
#define MAX_LINE     256

//SQL
sqlite3* db;


static int callback(void* NotUsed, int argc, char** argv, char** azColName) {
    int i;
    for (i = 0; i < argc; i++) {
        std::cout << azColName[i] << " = " << (argv[i] ? argv[i] : "NULL") << std::endl;
    }
    return 0;
}


// Function to check if a user exists in the database
bool userExists(int userId) {
    char sql[100];
    sqlite3_stmt* stmt;
    sprintf(sql, "SELECT COUNT(*) FROM Users WHERE user_id = %d", userId);


    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        return false;
    }

    int result = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = sqlite3_column_int(stmt, 0);
    }


    sqlite3_finalize(stmt);
    return result > 0;
}

float user1_balance = 100.0;




// Function to handle BUY command
void handleBuy(SOCKET clientSocket, char* command) {
    char stock_symbol[20];
    float stock_amount;
    float price_per_stock;
    int user_id;

    // Parse command
    if (sscanf(command, "BUY %s %f %f %d", stock_symbol, &stock_amount, &price_per_stock, &user_id) != 4) {
        send(clientSocket, "Invalid command format", 23, 0);
        return;
    }

}
    float total_cost = stock_amount * price_per_stock;

    // Check if user has enough balance and update balance if needed
    if (user_id == 1 && user1_balance >= total_cost) {

        user1_balance -= total_cost;
        updateStocksTable(stock_symbol, stock_amount, price_per_stock);

        // info message to client
        char response[MAX_LINE];
        sprintf(response, "200 OK\nBOUGHT: New balance: %.2f %s. USD balance $%.2f\n", stock_amount, stock_symbol, user1_balance);
        send(clientSocket, response, strlen(response), 0);
    }
    else {
        send(clientSocket, "Not enough balance or user doesn't exist\n", 42, 0);
    }
}
// Function to handle SELL command
void handleSell(SOCKET clientSocket, char* command) {
    char stock_symbol[20];
    float stock_amount;
    float price_per_stock;
    int user_id;

    // Parse 
    if (sscanf(command, "SELL %s %f %f %d", stock_symbol, &stock_amount, &price_per_stock, &user_id) != 4) {
        send(clientSocket, "Invalid command format", 23, 0);
        return;
    }

    //total
    float total_earnings = stock_amount * price_per_stock;

    // Deposit earnings to user balance (assuming user balance is a global variable)
    user1_balance += total_earnings;

    // Send success message to client
    char response[MAX_LINE];
    sprintf(response, "200 OK\nSOLD: New balance: %.1f %s. USD $%.2f\n", stock_amount, stock_symbol, user1_balance);
    send(clientSocket, response, strlen(response), 0);
}



// Function to handle LIST command
void handleList(SOCKET clientSocket) {
    sqlite3* db;
    int rc = sqlite3_open("stocks.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        send(clientSocket, "Database error", 14, 0);
        return;
    }

    const char* sql = "SELECT * FROM Stocks;";
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        send(clientSocket, "Database error", 14, 0);
        sqlite3_close(db);
        return;
    }

    //  response
    std::string response = "200 OK\nThe list of records in the Stocks database:\n";
    char row[MAX_LINE];

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        snprintf(row, sizeof(row), "%d %s %.1f %d\n", sqlite3_column_int(stmt, 0),
            sqlite3_column_text(stmt, 1), sqlite3_column_double(stmt, 2),
            sqlite3_column_int(stmt, 3));
        response += row;
    }

    sqlite3_finalize(stmt);

    // Send response to client
    send(clientSocket, response.c_str(), response.size(), 0);

    // Close 
    sqlite3_close(db);
}

// Function to handle BALANCE command
void handleBalance(SOCKET clientSocket) {
    // Open SQLite 
    sqlite3* db;
    int rc = sqlite3_open("stocks.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        send(clientSocket, "Database error", 14, 0);
        return;
    }

    // Query USD balance 
    const char* sql = "SELECT balance FROM Users WHERE user_id = 1;";
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        send(clientSocket, "Database error", 14, 0);
        sqlite3_close(db);
        return;
    }

    // Execute query and fetch result
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        double balance = sqlite3_column_double(stmt, 0);
        char response[MAX_LINE];
        snprintf(response, sizeof(response), "200 OK\nBalance for user 1: $%.2f", balance);
        send(clientSocket, response, strlen(response), 0);
    }
    else {
        send(clientSocket, "User not found", 14, 0);
    }

    sqlite3_finalize(stmt);

    // Close 
    sqlite3_close(db);
}

// Function to handle SHUTDOWN command
void handleShutdown(SOCKET clientSocket, SOCKET listenSocket) {
    send(clientSocket, "200 OK\n", 7, 0);
    closesocket(clientSocket);
    closesocket(listenSocket);
    WSACleanup();
    exit(0);
}

// Function to handle QUIT command
void handleQuit(SOCKET clientSocket) {
    send(clientSocket, "200 OK\n", 7, 0);
    closesocket(clientSocket);
}






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

    // Open database connection
    if (sqlite3_open("stocks.db", &db) != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    // Check if user exists before handling client requests
    if (!userExists(1)) {
        printf("User does not exist in the database. Exiting...\n");
        sqlite3_close(db);
        return 1;
    }

    // build address data structure 
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(SERVER_PORT);

    //setup passive open 
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
                // Handle client command
                if (strncmp(buf, "BUY", 3) == 0) {
                    handleBuy(clientSocket, buf);
                }
                else if (strncmp(buf, "SELL", 4) == 0) {
                    handleSell(clientSocket, buf);
                }
                else if (strncmp(buf, "LIST", 4) == 0) {
                    handleList(clientSocket);
                }
                else if (strncmp(buf, "BALANCE", 7) == 0) {
                    handleBalance(clientSocket);
                }
                else if (strncmp(buf, "SHUTDOWN", 8) == 0) {
                    handleShutdown(clientSocket);
                }
                else if (strncmp(buf, "QUIT", 4) == 0) {
                    handleQuit(clientSocket);
                }
                else {
                    printf("Invalid command\n");
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
    sqlite3_close(db);
    return 0;
}