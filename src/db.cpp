#include "db.h"
#include "sqlite3.h"
#include <iostream>
#include <chrono>
#include <ctime>

namespace indexit {

// --- SqliteStatement Implementation ---

SqliteStatement::SqliteStatement(sqlite3* db, const std::string& sql) {
    if (!db) throw SqliteException("Database connection is null");
    int rc = sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::string err = sqlite3_errmsg(db);
        throw SqliteException("Failed to prepare statement: " + err + " (SQL: " + sql + ")");
    }
}

SqliteStatement::~SqliteStatement() {
    if (stmt) {
        sqlite3_finalize(stmt);
    }
}

void SqliteStatement::bindInt64(int index, int64_t val) {
    int rc = sqlite3_bind_int64(stmt, index, val);
    if (rc != SQLITE_OK) throw SqliteException("Failed to bind integer: " + std::to_string(rc));
}

void SqliteStatement::bindInt(int index, int val) {
    int rc = sqlite3_bind_int(stmt, index, val);
    if (rc != SQLITE_OK) throw SqliteException("Failed to bind integer: " + std::to_string(rc));
}

void SqliteStatement::bindText(int index, const std::string& val) {
    int rc = sqlite3_bind_text(stmt, index, val.c_str(), static_cast<int>(val.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) throw SqliteException("Failed to bind text: " + std::to_string(rc));
}

void SqliteStatement::bindWText(int index, const std::wstring& val) {
    // Convert wide string to UTF-8
    int requiredSize = WideCharToMultiByte(CP_UTF8, 0, val.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (requiredSize > 0) {
        std::string utf8Str(static_cast<size_t>(requiredSize), '\0');
        if (!WideCharToMultiByte(CP_UTF8, 0, val.c_str(), -1, &utf8Str[0], requiredSize, nullptr, nullptr))
            throw SqliteException("Failed to convert wide text to UTF-8");
        utf8Str.resize(static_cast<size_t>(requiredSize - 1)); // exclude the terminating NUL
        int rc = sqlite3_bind_text(stmt, index, utf8Str.c_str(), static_cast<int>(utf8Str.size()), SQLITE_TRANSIENT);
        if (rc != SQLITE_OK) throw SqliteException("Failed to bind wide text: " + std::to_string(rc));
    } else {
        int rc = sqlite3_bind_text(stmt, index, "", 0, SQLITE_STATIC);
        if (rc != SQLITE_OK) throw SqliteException("Failed to bind empty text: " + std::to_string(rc));
    }
}

bool SqliteStatement::step() {
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) return true;
    if (rc == SQLITE_DONE) return false;
    throw SqliteException("Statement step error code: " + std::to_string(rc));
}

int64_t SqliteStatement::getInt64(int col) {
    return sqlite3_column_int64(stmt, col);
}

int SqliteStatement::getInt(int col) {
    return sqlite3_column_int(stmt, col);
}

std::string SqliteStatement::getText(int col) {
    const unsigned char* text = sqlite3_column_text(stmt, col);
    return text ? reinterpret_cast<const char*>(text) : "";
}

std::wstring SqliteStatement::getWText(int col) {
    const unsigned char* text = sqlite3_column_text(stmt, col);
    if (!text) return L"";
    const char* utf8 = reinterpret_cast<const char*>(text);
    int requiredSize = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, nullptr, 0);
    if (requiredSize > 0) {
        std::wstring wstr(static_cast<size_t>(requiredSize), L'\0');
        if (!MultiByteToWideChar(CP_UTF8, 0, utf8, -1, &wstr[0], requiredSize))
            throw SqliteException("Failed to convert UTF-8 text to wide string");
        wstr.resize(static_cast<size_t>(requiredSize - 1)); // exclude the terminating NUL
        return wstr;
    }
    return L"";
}

void SqliteStatement::reset() {
    int rc = sqlite3_reset(stmt);
    if (rc != SQLITE_OK) throw SqliteException("Failed to reset statement: " + std::to_string(rc));
    rc = sqlite3_clear_bindings(stmt);
    if (rc != SQLITE_OK) throw SqliteException("Failed to clear statement bindings: " + std::to_string(rc));
}

// --- SqliteTransaction Implementation ---

SqliteTransaction::SqliteTransaction(sqlite3* db) : db(db), committed(false) {
    if (!db) throw SqliteException("Database connection is null");
    char* errmsg = nullptr;
    int rc = sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK) {
        std::string msg = errmsg ? errmsg : "Error starting transaction";
        sqlite3_free(errmsg);
        throw SqliteException(msg);
    }
}

SqliteTransaction::~SqliteTransaction() {
    if (!committed && db) {
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
    }
}

void SqliteTransaction::commit() {
    if (!committed && db) {
        char* errmsg = nullptr;
        int rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &errmsg);
        if (rc != SQLITE_OK) {
            std::string msg = errmsg ? errmsg : "Error committing transaction";
            sqlite3_free(errmsg);
            throw SqliteException(msg);
        }
        committed = true;
    }
}

void SqliteTransaction::rollback() {
    if (!committed && db) {
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        committed = true; // prevent destructor from rolling back again
    }
}

// --- SqliteConnection Implementation ---

SqliteConnection::SqliteConnection(const std::string& path) {
    int rc = sqlite3_open(path.c_str(), &db);
    if (rc != SQLITE_OK) {
        std::string err = db ? sqlite3_errmsg(db) : "Unknown error opening database";
        if (db) sqlite3_close(db);
        db = nullptr;
        throw SqliteException("Failed to open SQLite database at '" + path + "': " + err);
    }
}

SqliteConnection::~SqliteConnection() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}

void SqliteConnection::execute(const std::string& sql) {
    char* errmsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK) {
        std::string msg = errmsg ? errmsg : "SQL execution error";
        sqlite3_free(errmsg);
        throw SqliteException("SQL Error (" + sql + "): " + msg);
    }
}

// --- DatabaseManager Implementation ---

DatabaseManager::DatabaseManager(const std::string& dbPath) : conn_(dbPath) {
    initSchema();
}

void DatabaseManager::initSchema() {
    conn_.execute(
        "CREATE TABLE IF NOT EXISTS documents ("
        "  id INTEGER PRIMARY KEY,"
        "  path TEXT NOT NULL UNIQUE,"
        "  size INTEGER,"
        "  mtime INTEGER,"
        "  format INTEGER,"
        "  doc_len INTEGER"
        ");"
        "CREATE INDEX IF NOT EXISTS idx_documents_path ON documents(path);"
        "CREATE TABLE IF NOT EXISTS cache_manifest ("
        "  path TEXT NOT NULL UNIQUE,"
        "  size INTEGER,"
        "  created TEXT"
        ");"
        "CREATE INDEX IF NOT EXISTS idx_manifest_path ON cache_manifest(path);"
        "CREATE TABLE IF NOT EXISTS search_history ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  query TEXT NOT NULL,"
        "  ts TEXT NOT NULL,"
        "  result_count INTEGER"
        ");"
    );
}

void DatabaseManager::saveDocuments(const DocumentStore& store) {
    SqliteTransaction txn(conn_.get());
    SqliteStatement stmt(conn_.get(),
        "INSERT OR REPLACE INTO documents (id, path, size, mtime, format, doc_len) "
        "VALUES (?, ?, ?, ?, ?, ?);"
    );

    const auto& docs = store.getAllDocuments();
    for (const auto& kv : docs) {
        const auto& d = kv.second;
        stmt.reset();
        stmt.bindInt64(1, d.id);
        stmt.bindWText(2, d.path);
        stmt.bindInt64(3, d.fileSize);
        stmt.bindInt64(4, d.lastWriteTime);
        stmt.bindInt(5, static_cast<int>(d.format));
        stmt.bindInt(6, d.length);
        stmt.step();
    }
    txn.commit();
}

void DatabaseManager::loadDocuments(DocumentStore& store) {
    SqliteStatement stmt(conn_.get(),
        "SELECT id, path, size, mtime, format, doc_len FROM documents ORDER BY id ASC;"
    );

    while (stmt.step()) {
        DocMetadata meta;
        meta.id = static_cast<uint64_t>(stmt.getInt64(0));
        meta.path = stmt.getWText(1);
        meta.fileSize = static_cast<uint64_t>(stmt.getInt64(2));
        meta.lastWriteTime = static_cast<uint64_t>(stmt.getInt64(3));
        meta.format = static_cast<DocumentFormat>(stmt.getInt(4));
        meta.length = static_cast<uint32_t>(stmt.getInt(5));

        store.addExistingDocument(meta);
    }
}

void DatabaseManager::recordManifest(const std::string& path, uint64_t size, const std::string& created) {
    SqliteStatement stmt(conn_.get(),
        "INSERT OR REPLACE INTO cache_manifest (path, size, created) VALUES (?, ?, ?);"
    );
    stmt.bindText(1, path);
    stmt.bindInt64(2, size);
    stmt.bindText(3, created);
    stmt.step();
}

void DatabaseManager::recordSearchHistory(const std::string& query, int resultCount) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    char buf[64];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
             st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

    SqliteStatement stmt(conn_.get(),
        "INSERT INTO search_history (query, ts, result_count) VALUES (?, ?, ?);"
    );
    stmt.bindText(1, query);
    stmt.bindText(2, buf);
    stmt.bindInt(3, resultCount);
    stmt.step();
}

std::vector<SearchHistoryRecord> DatabaseManager::getSearchHistory(int limit) {
    std::vector<SearchHistoryRecord> records;
    SqliteStatement stmt(conn_.get(),
        "SELECT id, query, ts, result_count FROM search_history ORDER BY id DESC LIMIT ?;"
    );
    stmt.bindInt(1, limit);
    while (stmt.step()) {
        SearchHistoryRecord rec;
        rec.id = stmt.getInt(0);
        rec.query = stmt.getText(1);
        rec.timestamp = stmt.getText(2);
        rec.resultCount = stmt.getInt(3);
        records.push_back(rec);
    }
    return records;
}

} // namespace indexit
