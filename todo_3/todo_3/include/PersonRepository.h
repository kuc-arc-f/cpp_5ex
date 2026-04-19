#pragma once
#include "Database.h"
#include "Person.h"
#include <vector>

class PersonRepository {
public:
    explicit PersonRepository(Database& db);

    void        createTable();

    // Create
    bool        insert(const Person& p);

    // Read
    std::vector<Person> findAll();
    bool                findById(int id, Person& out);

    // Update
    bool        update(const Person& p);

    // Delete
    bool        remove(int id);

private:
    Database& db_;
};
