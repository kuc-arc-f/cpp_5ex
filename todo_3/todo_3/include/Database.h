#pragma once
#include "sqlite3.h"
#include <string>
#include <stdexcept>

class Database {
public:
    explicit Database(const std::string& path);
    ~Database();

    sqlite3* get() const { return db_; }

    // コピー禁止
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

private:
    sqlite3* db_ = nullptr;
};
