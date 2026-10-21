#include "RAII.hpp"
// #include "sqlite3.h"

namespace indexit {

SqliteConnection::SqliteConnection(const std::string& path) {
    db = nullptr;
    // sqlite3_open(path.c_str(), &db);
}

SqliteConnection::~SqliteConnection() {
    if (db) {
        // sqlite3_close(db);
    }
}

} // namespace indexit
