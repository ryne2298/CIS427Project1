#include <iostream>
#include <sqlite3.h>

using namespace std;

int createDB(const char* s);
int createTable(const char* s);

int main()
{
    const char* dir = "C:\\Users\\Yuliya\\source\\repos\\CIS427Project1\\Project1\\DB\\Stock.db";
    sqlite3* DB;

    int exit = 0;
    exit = sqlite3_open(dir, &DB);
    if (exit) {
        cerr << "Error open DB " << sqlite3_errmsg(DB) << endl;
        return (-1);
    }
    else {
        cout << "Opened Database Successfully!" << endl;
    }
    sqlite3_close(DB);

    createDB(dir);
    createTable(dir);

    return 0;
}

int createDB(const char* s) {
    sqlite3* DB;
    int exit = sqlite3_open(s, &DB);

    sqlite3_close(DB);

    return 0;
}

int createTable(const char* s) {
    sqlite3* DB;
    char* messageError;

    string sql = "CREATE TABLE IF NOT EXISTS USERS("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
        "EMAIL TEXT NOT NULL, "
        "FIRST_NAME TEXT, "
        "LAST_NAME TEXT, "
        "USER_NAME TEXT NOT NULL, "
        "PASSWORD TEXT, "
        "USD_BALANCE REAL NOT NULL);";

    try {
        int exit = 0;
        exit = sqlite3_open(s, &DB);

        exit = sqlite3_exec(DB, sql.c_str(), NULL, 0, &messageError);
        if (exit != SQLITE_OK) {
            cerr << "Error Create Table" << endl;
            sqlite3_free(messageError);
        }
        else
            cout << "Table Created Successfully" << endl;
        sqlite3_close(DB);
    }
    catch (const exception& e) {
        cerr << e.what();
    }

    return 0;
}

