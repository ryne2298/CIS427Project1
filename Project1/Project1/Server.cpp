#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <unordered_map>
#include <string.h>
#include <sqlite3.h>
#include <thread>


using namespace std;

#define SERVER_PORT  8080
#define MAX_PENDING  5
#define MAX_LINE     256

//SQL
sqlite3* db;
unordered_map<string, string> userCredentials = { {"user1", "pass1"}, {"user2", "pass2"}, {"user3", "pass3"}, {"user4", "pass4"} };
unordered_map<SOCKET, string> activeUsers; // Mapping sockets to user IDs for active sessions
vector<SOCKET> allClientSockets; // For tracking all connected client sockets
bool updateUserBalance(const std::string& userId, float depositAmount);

//current user ID
string currentUserId;

static int callback(void* NotUsed, int argc, char** argv, char** azColName) {
    int i;
    for (i = 0; i < argc; i++) {
        std::cout << azColName[i] << " = " << (argv[i] ? argv[i] : "NULL") << std::endl;
    }
    return 0;
}

//modified due to using iostream not sstream
void handleLogin(SOCKET clientSocket, char* command) {
    std::string cmdStr(command); 
    size_t posUserId = cmdStr.find(' ') + 1; // Find end of "LOGIN "
    size_t posSpaceAfterUserId = cmdStr.find(' ', posUserId); 
    std::string userId = cmdStr.substr(posUserId, posSpaceAfterUserId - posUserId); // Extract userId

    size_t posPassword = posSpaceAfterUserId + 1; // Start of password
    std::string password = cmdStr.substr(posPassword); // Extract password

    if (userCredentials.find(userId) != userCredentials.end() && userCredentials[userId] == password) {
        send(clientSocket, "200 OK\n", 7, 0);
        currentUserId = userId;
    }
    else {
        send(clientSocket, "403 Wrong UserID or Password\n", 29, 0);
    }
}

// Function to check if a user exists in the database
bool userExists(int userId) {
    char sql[100];
    sqlite3_stmt* stmt;
    sprintf_s(sql, sizeof(sql), "SELECT COUNT(*) FROM Users WHERE user_id = %d", userId);


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


void updateStocksTable(const char* stock_symbol, float stock_amount, float price_per_stock) {
    char* errMsg = nullptr;
    char sql[1024];


    // update the stock amount and price if the symbol exists,
    // or insert a new record if it doesn't
    sprintf_s(sql, sizeof(sql),
        "INSERT INTO stocks (symbol, amount, price) VALUES ('%s', %f, %f) "
        "ON CONFLICT(symbol) DO UPDATE SET amount = amount + %f, price = %f;",
        stock_symbol, stock_amount, price_per_stock, stock_amount, price_per_stock);

    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }
    else {
        printf("Stock table updated successfully\n");
    }
}






// Function to handle BUY command
void handleBuy(SOCKET clientSocket, char* command)
{
    char stock_symbol[20];
    float stock_amount;
    float price_per_stock;
    int user_id;

    // Parse command
    if (sscanf_s(command, "BUY %s %f %f %d", stock_symbol, (unsigned)_countof(stock_symbol), &stock_amount, &price_per_stock, &user_id) != 4) {
        {
            send(clientSocket, "Invalid command format", 23, 0);
            return;
        }


        float total_cost = stock_amount * price_per_stock;

        // Check if user has enough balance and update balance if needed
        if (user_id == 1 && user1_balance >= total_cost) {

            user1_balance -= total_cost;
            updateStocksTable(stock_symbol, stock_amount, price_per_stock);

            // info message to client
            char response[MAX_LINE];
            sprintf_s(response, sizeof(response), "200 OK\nBOUGHT: New balance: %.2f %s. USD balance $%.2f\n", stock_amount, stock_symbol, user1_balance);
            send(clientSocket, response, static_cast<int>(strlen(response)), 0);

        }
        else {
            send(clientSocket, "Not enough balance or user doesn't exist\n", 42, 0);
        }

    }

}


// Function to handle SELL command
void handleSell(SOCKET clientSocket, char* command)
{
    char stock_symbol[20];
    float stock_amount;
    float price_per_stock;
    int user_id;

    // Parse 
    if (sscanf_s(command, "SELL %s %f %f %d", stock_symbol, (unsigned)_countof(stock_symbol), &stock_amount, &price_per_stock, &user_id) != 4)
    {

        {
            send(clientSocket, "Invalid command format", 23, 0);
            return;
        }

        //total
        float total_earnings = stock_amount * price_per_stock;

        // Deposit earnings to user balance (assuming user balance is a global variable)
        user1_balance += total_earnings;

        // Send success message to client
        char response[MAX_LINE];
        sprintf_s(response, sizeof(response), "200 OK\nSOLD: %.1f %s. USD balance: $%.2f", stock_amount, stock_symbol, user1_balance);
        send(clientSocket, response, static_cast<int>(strlen(response)), 0);

    }

}




// Function to handle LIST command
void handleList(SOCKET clientSocket, const std::string& userId) {
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
        send(clientSocket, response, static_cast<int>(strlen(response)), 0);

    }
    else {
        send(clientSocket, "User not found", 14, 0);
    }

    sqlite3_finalize(stmt);

    // Close 
    sqlite3_close(db);
}

// Function to handle SHUTDOWN command
void handleShutdown(SOCKET clientSocket, const std::string& userId) {
    if (userId != "root") {
        send(clientSocket, "Permission denied\n", 18, 0);
        return;
    }

    // Broadcast message to all connected clients before shutting down
    for (auto& socket : allClientSockets) {  
        send(socket, "Server is shutting down\n", 24, 0);
        closesocket(socket);
    }

    // Shutdown server operations
    WSACleanup();
    exit(0);  
}

// Function to handle QUIT command
void handleQuit(SOCKET clientSocket) {
    send(clientSocket, "200 OK\n", 7, 0);
    closesocket(clientSocket);
}

void handleWho(SOCKET clientSocket, const std::string& userId) {
    if (userId != "root") {
        send(clientSocket, "Permission denied\n", 18, 0);
        return;
    }

    std::string response = "200 OK\nThe list of the active users:\n";
    for (const auto& user : activeUsers) {  
        response += user.first + " " + user.second + "\n";
    }

    send(clientSocket, response.c_str(), response.length(), 0);
}

void handleLookup(SOCKET clientSocket, const std::string& userId, char* command) {
    std::string stockName = command + 7;
    std::string sql = "SELECT * FROM Stocks WHERE owner = '" + userId + "' AND symbol LIKE '%" + stockName + "%';";
}


//added functionality due to undefineed functions
bool updateUserBalance(const std::string& userId, float depositAmount) {
    char* errMsg = nullptr;
    char sql[1024];

    sprintf_s(sql, sizeof(sql), "UPDATE Users SET balance = balance + %f WHERE userId = '%s';", depositAmount, userId.c_str());

    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}
//added due to logic
float getCurrentBalance(const std::string& userId) {
    sqlite3_stmt* stmt = nullptr;
    float balance = 100.0; // Default to 100.0 if not found
    const char* sqlQuery = "SELECT balance FROM Users WHERE userId = ?";

    if (sqlite3_prepare_v2(db, sqlQuery, -1, &stmt, nullptr) == SQLITE_OK &&
        sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_STATIC) == SQLITE_OK &&
        sqlite3_step(stmt) == SQLITE_ROW) {
        balance = static_cast<float>(sqlite3_column_double(stmt, 0));
    }

    sqlite3_finalize(stmt);
    return balance;
}

void handleDeposit(SOCKET clientSocket, const std::string& userId, char* command) {
    float depositAmount;
    if (sscanf_s(command, "DEPOSIT %f", &depositAmount) == 1) {
        // Update user balance
        float newBalance = updateUserBalance(userId, depositAmount);

        //Output
        char response[MAX_LINE];
        snprintf(response, sizeof(response), "Deposit successfully. New balance $%.2f\n", newBalance);
        send(clientSocket, response, strlen(response), 0);
    }
    else {
        send(clientSocket, "Invalid command format\n", 22, 0);
    }
}

void handleLogout(SOCKET clientSocket, const std::string& userId) {
    if (userId.empty()) {
        send(clientSocket, "403 No User Logged In\n", 22, 0);
        return;
    }

    // Remove the user from the active users map
    activeUsers.erase(clientSocket);

    // closesocket(clientSocket);

    send(clientSocket, "200 OK\n", 7, 0);
}





int main() {
    WSADATA wsaData;
    SOCKET listenSocket = INVALID_SOCKET;
    struct sockaddr_in sin;
    int addr_len;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Error at WSAStartup()\n";
        return 1;
    }

    // Open database connection
    if (sqlite3_open("stock.db", &db) != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << "\n";
        return 1;
    }

    // Check if user exists before handling client requests
    if (!userExists(1)) {
        std::cerr << "User does not exist in the database. Exiting...\n";
        sqlite3_close(db);
        return 1;
    }

    // Build address data structure
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(SERVER_PORT);

    // Setup passive open
    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "Error at socket(): " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    if (bind(listenSocket, (struct sockaddr*)&sin, sizeof(sin)) == SOCKET_ERROR) {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << "\n";
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    if (listen(listenSocket, MAX_PENDING) == SOCKET_ERROR) {
        std::cerr << "Listen failed with error: " << WSAGetLastError() << "\n";
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    while (true) {
        addr_len = sizeof(sin);
        SOCKET clientSocket = accept(listenSocket, (struct sockaddr*)&sin, (int*)&addr_len);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed: " << WSAGetLastError() << "\n";
            continue;
        }

        // Create a thread for each connection and use a lambda function as the thread function
        std::thread([clientSocket]() {
            char buf[MAX_LINE];
            int buf_len;

            while (true) {
                buf_len = recv(clientSocket, buf, sizeof(buf), 0);
                if (buf_len > 0) {
                    buf[buf_len] = '\0';


                    // Handle client command
                    if (strncmp(buf, "BUY", 3) == 0) {
                        handleBuy(clientSocket, buf);
                    }
                    else if (strncmp(buf, "SELL", 4) == 0) {
                        handleSell(clientSocket, buf);
                    }
                    else if (strncmp(buf, "LIST", 4) == 0) {
                        handleList(clientSocket, currentUserId);
                    }
                    else if (strncmp(buf, "BALANCE", 7) == 0) {
                        handleBalance(clientSocket);
                    }
                    else if (strncmp(buf, "SHUTDOWN", 8) == 0) {
                        handleShutdown(clientSocket, currentUserId);
                    }
                    else if (strncmp(buf, "LOGIN", 5) == 0) {
                        handleLogin(clientSocket, buf);
                    }
                    else if (strncmp(buf, "WHO", 3) == 0) {
                        handleWho(clientSocket, currentUserId);
                    }
                    else if (strncmp(buf, "QUIT", 4) == 0) {
                        handleQuit(clientSocket);
                    }
                    else if (strncmp(buf, "LOOKUP", 6) == 0) {
                        handleLookup(clientSocket, currentUserId, buf);
                    }
                    else if (strncmp(buf, "DEPOSIT", 7) == 0) {
                        handleDeposit(clientSocket, currentUserId, buf);
                    }

                    if (strncmp(buf, "QUIT", 4) == 0) {
                        handleQuit(clientSocket);
                        break;
                    }
                    else {
                        printf("Invalid command\n");
                    }

                }
                else if (buf_len == 0) {
                    std::cout << "Connection closing...\n";
                    break; 
                }
                else {
                    std::cerr << "Recv failed with error: " << WSAGetLastError() << "\n";
                    break; 
                }
            }
            closesocket(clientSocket); 
        }).detach(); // Allow it to run independently
    }

    // Cleanup
    closesocket(listenSocket);
    WSACleanup();
    sqlite3_close(db);
    return 0;
}