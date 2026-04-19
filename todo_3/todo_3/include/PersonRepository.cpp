#include "PersonRepository.h"
#include <iostream>

PersonRepository::PersonRepository(Database& db) : db_(db) {}

// ─── テーブル作成 ────────────────────────────────────────────
void PersonRepository::createTable() {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS persons ("
        "  id    INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  name  TEXT    NOT NULL,"
        "  age   INTEGER NOT NULL,"
        "  email TEXT    NOT NULL UNIQUE"
        ");";

    char* errmsg = nullptr;
    if (sqlite3_exec(db_.get(), sql, nullptr, nullptr, &errmsg) != SQLITE_OK) {
        std::string err = errmsg;
        sqlite3_free(errmsg);
        throw std::runtime_error("createTable failed: " + err);
    }
}

// ─── INSERT ──────────────────────────────────────────────────
bool PersonRepository::insert(const Person& p) {
    const char* sql =
        "INSERT INTO persons (name, age, email) VALUES (?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.get(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_text(stmt, 1, p.name.c_str(),  -1, SQLITE_TRANSIENT);
    sqlite3_bind_int (stmt, 2, p.age);
    sqlite3_bind_text(stmt, 3, p.email.c_str(), -1, SQLITE_TRANSIENT);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

// ─── SELECT ALL ──────────────────────────────────────────────
std::vector<Person> PersonRepository::findAll() {
    std::vector<Person> result;
    const char* sql = "SELECT id, name, age, email FROM persons;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.get(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        return result;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Person p;
        p.id    = sqlite3_column_int (stmt, 0);
        p.name  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        p.age   = sqlite3_column_int (stmt, 2);
        p.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        result.push_back(p);
    }
    sqlite3_finalize(stmt);
    return result;
}

// ─── SELECT BY ID ────────────────────────────────────────────
bool PersonRepository::findById(int id, Person& out) {
    const char* sql =
        "SELECT id, name, age, email FROM persons WHERE id = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.get(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(stmt, 1, id);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        out.id    = sqlite3_column_int (stmt, 0);
        out.name  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        out.age   = sqlite3_column_int (stmt, 2);
        out.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        found = true;
    }
    sqlite3_finalize(stmt);
    return found;
}

// ─── UPDATE ──────────────────────────────────────────────────
bool PersonRepository::update(const Person& p) {
    const char* sql =
        "UPDATE persons SET name = ?, age = ?, email = ? WHERE id = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.get(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_text(stmt, 1, p.name.c_str(),  -1, SQLITE_TRANSIENT);
    sqlite3_bind_int (stmt, 2, p.age);
    sqlite3_bind_text(stmt, 3, p.email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int (stmt, 4, p.id);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

// ─── DELETE ──────────────────────────────────────────────────
bool PersonRepository::remove(int id) {
    const char* sql = "DELETE FROM persons WHERE id = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.get(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(stmt, 1, id);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}
