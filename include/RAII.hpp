#pragma once
#include <windows.h>
#include <string>
#include "Exceptions.hpp"
#include <chrono>
#include <stdexcept>
#include <cstdint>

struct sqlite3;
struct sqlite3_stmt;

namespace indexit {

class FileHandle {
    HANDLE hFile;
public:
    FileHandle(const std::wstring& path, DWORD access, DWORD share, DWORD creation) {
        hFile = CreateFileW(path.c_str(), access, share, nullptr, creation, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) {
            throw IOException("Failed to open file");
        }
    }
    ~FileHandle() { if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile); }
    HANDLE get() const { return hFile; }
    
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;
};

class MappedFile {
    FileHandle file;
    HANDLE hMap;
    void* pBuf;
    size_t size;
public:
    MappedFile(const std::wstring& path) 
        : file(path, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING), hMap(nullptr), pBuf(nullptr) {
        LARGE_INTEGER li;
        if (!GetFileSizeEx(file.get(), &li)) throw IOException("GetFileSizeEx failed");
        size = li.QuadPart;
        if (size > 0) {
            hMap = CreateFileMappingW(file.get(), nullptr, PAGE_READONLY, 0, 0, nullptr);
            if (!hMap) throw IOException("CreateFileMapping failed");
            pBuf = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
            if (!pBuf) throw IOException("MapViewOfFile failed");
        }
    }
    ~MappedFile() {
        if (pBuf) UnmapViewOfFile(pBuf);
        if (hMap) CloseHandle(hMap);
    }
    const char* data() const { return static_cast<const char*>(pBuf); }
    size_t length() const { return size; }
};

class ScopedTimer {
    using Clock = std::chrono::high_resolution_clock;
    Clock::time_point start;
    std::string name;
public:
    explicit ScopedTimer(std::string n = "") : name(std::move(n)), start(Clock::now()) {}
    ~ScopedTimer() {}
    long long elapsedMs() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start).count();
    }
    double elapsedUs() const {
        return static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - start).count());
    }
    double elapsedSec() const {
        return std::chrono::duration<double>(Clock::now() - start).count();
    }
    void reset() {
        start = Clock::now();
    }
    const std::string& getName() const { return name; }
};

class SqliteException : public std::runtime_error {
public:
    explicit SqliteException(const std::string& msg) : std::runtime_error(msg) {}
};

class SqliteStatement {
private:
    sqlite3_stmt* stmt = nullptr;
public:
    SqliteStatement(sqlite3* db, const std::string& sql);
    ~SqliteStatement();
    SqliteStatement(const SqliteStatement&) = delete;
    SqliteStatement& operator=(const SqliteStatement&) = delete;

    void bindInt64(int index, int64_t val);
    void bindInt(int index, int val);
    void bindText(int index, const std::string& val);
    void bindWText(int index, const std::wstring& val);
    bool step();
    int64_t getInt64(int col);
    int getInt(int col);
    std::string getText(int col);
    std::wstring getWText(int col);
    void reset();
    sqlite3_stmt* get() const { return stmt; }
};

class SqliteTransaction {
private:
    sqlite3* db;
    bool committed = false;
public:
    explicit SqliteTransaction(sqlite3* db);
    ~SqliteTransaction();
    SqliteTransaction(const SqliteTransaction&) = delete;
    SqliteTransaction& operator=(const SqliteTransaction&) = delete;

    void commit();
    void rollback();
};

class SqliteConnection {
private:
    sqlite3* db = nullptr;
public:
    explicit SqliteConnection(const std::string& path);
    ~SqliteConnection();
    SqliteConnection(const SqliteConnection&) = delete;
    SqliteConnection& operator=(const SqliteConnection&) = delete;

    sqlite3* get() const { return db; }
    void execute(const std::string& sql);
};

} // namespace indexit
