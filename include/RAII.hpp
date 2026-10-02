#pragma once
#include <windows.h>
#include <string>
#include "Exceptions.hpp"
#include <chrono>

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
    ScopedTimer(std::string n) : name(std::move(n)), start(Clock::now()) {}
    ~ScopedTimer() {
        // can be used for profiling
    }
    long long elapsedMs() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start).count();
    }
};

class SqliteConnection {
    struct sqlite3* db;
public:
    SqliteConnection(const std::string& path);
    ~SqliteConnection();
    struct sqlite3* get() const { return db; }
};

} // namespace indexit
