#include "Database.h"
#include <iostream>

Database::Database(const std::string& name)
    : db(nullptr),dbName(name){}

Database::~Database(){
    close();
}

bool Database::open(){
    return sqlite3_open(dbName.c_str(),&db) == SQLITE_OK;
}

void  Database::close(){
    if (db) sqlite3_close(db);
    db = nullptr;
}

// 銘柄一覧取得
std::vector<Brand> Database::getBrands(){
    std::vector<Brand>list;
    sqlite3_stmt* stmt;

    
}