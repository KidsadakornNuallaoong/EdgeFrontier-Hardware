#if !defined(DB_HELPER_HPP)
#define DB_HELPER_HPP

#include <string>
#include <sqlite3.h>
#include <iostream>
#include <vector>
#include <nlohmann/json.hpp>

class DBHelper {
private:
    std::string db_name;
    sqlite3* db;

    // Utility to escape single quotes for SQL
    std::string escape_sql(const std::string& input) const {
        std::string output;
        for (char c : input) {
            if (c == '\'') {
                output += "''";
            } else {
                output += c;
            }
        }
        return output;
    }

public:
    // Constructor
    explicit DBHelper(const std::string& db_name) : db_name(db_name), db(nullptr) {
        int rc = sqlite3_open(db_name.c_str(), &db);
        if (rc) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            db = nullptr;
        }
    }

    // Destructor
    ~DBHelper() {
        if (db) {
            sqlite3_close(db);
            db = nullptr;
        }
    }

    // Make class non-copyable to prevent resource duplication
    DBHelper(const DBHelper&) = delete;
    DBHelper& operator=(const DBHelper&) = delete;

    // Create a table
    void create_table(const std::string& table_name, const std::vector<std::string>& columns) {
        if (!db) return;

        // Build the SQL query
        std::string sql = "CREATE TABLE IF NOT EXISTS " + escape_sql(table_name) + " (";
        for (auto it = columns.begin(); it != columns.end(); ++it) {
            sql += escape_sql(*it);
            if (std::next(it) != columns.end()) {
                sql += ", ";
            }
        }
        sql += ");";

        // Execute the query
        char* zErrMsg = nullptr;
        int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &zErrMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg);
        }
    }

    // Insert data into a table
    void insert_data(const std::string& table_name, const nlohmann::json& data) {
        if (!db) return;

        // Build the SQL query with column names
        std::string sql = "INSERT INTO " + escape_sql(table_name) + " (";
        std::string values = "VALUES (";

        for (auto it = data.begin(); it != data.end(); ++it) {
            sql += escape_sql(it.key()); // Add column name
            if (it.value().is_string()) {
                values += "'" + escape_sql(it.value().get<std::string>()) + "'"; // Add string value
            } else if (it.value().is_number()) {
                values += std::to_string(it.value().get<double>()); // Add numeric value
            } else {
                values += "NULL"; // Handle unsupported types
            }

            if (std::next(it) != data.end()) {
                sql += ", ";
                values += ", ";
            }
        }
        sql += ") " + values + ");";

        // Execute the query
        char* zErrMsg = nullptr;
        int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &zErrMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg);
        }
    }

    // Select data from a table
    void select_data(const std::string& table_name, const std::string& columns = "*") {
        if (!db) return;

        std::string sql = "SELECT " + escape_sql(columns) + " FROM " + escape_sql(table_name) + ";";
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
            return;
        }

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            for (int i = 0; i < sqlite3_column_count(stmt); ++i) {
                const unsigned char* text = sqlite3_column_text(stmt, i);
                std::cout << (text ? reinterpret_cast<const char*>(text) : "NULL") << " ";
            }
            std::cout << std::endl;
        }
        sqlite3_finalize(stmt);
    }

    void delete_data(const std::string& table_name, const std::string& condition) {
        if (!db) return;

        std::string sql = "DELETE FROM " + escape_sql(table_name) + " WHERE " + escape_sql(condition) + ";";
        char* zErrMsg = nullptr;
        int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &zErrMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg);
        }
    }
};

#endif // DB_HELPER_HPP