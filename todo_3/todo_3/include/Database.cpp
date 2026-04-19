#include "Database.h"

Database::Database(const std::string& path) {
    if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
        std::string err = sqlite3_errmsg(db_);
        sqlite3_close(db_);
        throw std::runtime_error("DB open failed: " + err);
    }
}

Database::~Database() {
    if (db_) sqlite3_close(db_);
}
