#include "BinaryIndexCache.hpp"
#include "CppInvertedIndex.hpp"
#include <windows.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cwchar>

extern "C" {
#include "winfs.h"
}

namespace indexit {

#pragma pack(push, 1)
struct IndexCacheHeader {
    char magic[8];        // "INDEXIT\0"
    uint32_t version;     // 1
    uint32_t crc32;       // CRC32 of all payload bytes following header
    uint64_t doc_count;
    uint64_t term_count;
    double avg_doc_length;
};
#pragma pack(pop)

static const char MAGIC_STR[8] = {'I', 'N', 'D', 'E', 'X', 'I', 'T', '\0'};
static const uint32_t FORMAT_VERSION = 1;

static std::string toUtf8(const std::wstring& value) {
    if (value.empty()) return {};
    int length = WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()),
                                    nullptr, 0, nullptr, nullptr);
    if (!length) return {};
    std::string result(length, '\0');
    WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()),
                        &result[0], length, nullptr, nullptr);
    return result;
}

// Standard CRC-32 (IEEE 802.3)
uint32_t BinaryIndexCache::computeCrc32(const void* data, size_t length) {
    static uint32_t crc_table[256];
    static bool table_computed = false;
    if (!table_computed) {
        for (uint32_t i = 0; i < 256; i++) {
            uint32_t c = i;
            for (int j = 0; j < 8; j++) {
                if (c & 1) c = 0xEDB88320L ^ (c >> 1);
                else c = c >> 1;
            }
            crc_table[i] = c;
        }
        table_computed = true;
    }

    uint32_t crc = 0xFFFFFFFFL;
    const uint8_t* p = static_cast<const uint8_t*>(data);
    for (size_t i = 0; i < length; i++) {
        crc = crc_table[(crc ^ p[i]) & 0xFF] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFFL;
}

std::wstring BinaryIndexCache::getDefaultCacheDir() {
    wchar_t localAppData[MAX_PATH];
    DWORD len = GetEnvironmentVariableW(L"LOCALAPPDATA", localAppData, MAX_PATH);
    if (len > 0 && len < MAX_PATH) {
        return std::wstring(localAppData) + L"\\IndexIt";
    }
    return L".\\.indexit_cache";
}

bool BinaryIndexCache::ensureDirectoryExists(const std::wstring& dir) {
    DWORD attr = GetFileAttributesW(dir.c_str());
    if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
        return true;
    }
    // Recursive create
    for (size_t i = 1; i < dir.size(); i++) {
        if (dir[i] == L'\\' || dir[i] == L'/') {
            std::wstring sub = dir.substr(0, i);
            CreateDirectoryW(sub.c_str(), NULL);
        }
    }
    return CreateDirectoryW(dir.c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS;
}

bool BinaryIndexCache::checkDiskSpace(const std::wstring& cacheDir, uint64_t minBytesRequired) {
    ULARGE_INTEGER freeBytes, totalBytes, totalFree;
    if (GetDiskFreeSpaceExW(cacheDir.c_str(), &freeBytes, &totalBytes, &totalFree)) {
        if (freeBytes.QuadPart < minBytesRequired) {
            std::cerr << "[WARN] Insufficient disk space in cache directory: "
                      << (freeBytes.QuadPart / (1024 * 1024)) << " MB free (threshold: "
                      << (minBytesRequired / (1024 * 1024)) << " MB). Continuing in memory-only mode.\n";
            return false;
        }
        return true;
    }
    return true; // Default allow if query fails
}

uint64_t BinaryIndexCache::getAvailableDiskSpace(const std::wstring& cacheDir) {
    ULARGE_INTEGER freeBytes, totalBytes, totalFree;
    if (GetDiskFreeSpaceExW(cacheDir.c_str(), &freeBytes, &totalBytes, &totalFree)) {
        return freeBytes.QuadPart;
    }
    return 0;
}

uint64_t BinaryIndexCache::getCacheFileSize(const std::wstring& cacheDir) {
    std::wstring cacheFile = cacheDir + L"\\index.cache";
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (GetFileAttributesExW(cacheFile.c_str(), GetFileExInfoStandard, &fad)) {
        LARGE_INTEGER li;
        li.HighPart = fad.nFileSizeHigh;
        li.LowPart = fad.nFileSizeLow;
        return li.QuadPart;
    }
    return 0;
}

static std::wstring uintToWstring(uint64_t val) {
    if (val == 0) return L"0";
    std::wstring s;
    while (val > 0) {
        s.push_back(static_cast<wchar_t>(L'0' + (val % 10)));
        val /= 10;
    }
    std::reverse(s.begin(), s.end());
    return s;
}

bool BinaryIndexCache::save(const std::wstring& cacheDir, const CppInvertedIndex& index) {
    if (!ensureDirectoryExists(cacheDir)) return false;
    if (!checkDiskSpace(cacheDir)) return false;

    std::wstring finalPath = cacheDir + L"\\index.cache";
    std::wstring tempPath = cacheDir + L"\\index.cache.tmp." + uintToWstring(GetCurrentProcessId());

    std::vector<uint8_t> payload;

    // 1. Serialize DocumentStore
    const auto& docs = index.getDocumentStore().getAllDocuments();
    for (const auto& kv : docs) {
        const auto& d = kv.second;
        // doc_id, file_size, last_write_time, doc_length, format, path_len, path
        uint64_t docId = d.id;
        uint64_t fsize = d.fileSize;
        uint64_t mtime = d.lastWriteTime;
        uint32_t dlen = d.length;
        uint32_t fmt = static_cast<uint32_t>(d.format);
        uint32_t pathChars = static_cast<uint32_t>(d.path.size());

        auto appendData = [&](const void* ptr, size_t sz) {
            const uint8_t* bytePtr = static_cast<const uint8_t*>(ptr);
            payload.insert(payload.end(), bytePtr, bytePtr + sz);
        };

        appendData(&docId, sizeof(docId));
        appendData(&fsize, sizeof(fsize));
        appendData(&mtime, sizeof(mtime));
        appendData(&dlen, sizeof(dlen));
        appendData(&fmt, sizeof(fmt));
        appendData(&pathChars, sizeof(pathChars));
        if (pathChars > 0) {
            appendData(d.path.data(), pathChars * sizeof(wchar_t));
        }
    }

    // 2. Serialize Postings from exactIndex_
    uint64_t termCount = 0;
    if (index.getExactIndex()) {
        termCount = index.getExactIndex()->size();
        index.getExactIndex()->forEachTerm([&](const std::string& term, const std::vector<Posting>& postings) {
            uint16_t tlen = static_cast<uint16_t>(term.size());
            uint32_t pcount = static_cast<uint32_t>(postings.size());

            auto appendData = [&](const void* ptr, size_t sz) {
                const uint8_t* bytePtr = static_cast<const uint8_t*>(ptr);
                payload.insert(payload.end(), bytePtr, bytePtr + sz);
            };

            appendData(&tlen, sizeof(tlen));
            if (tlen > 0) {
                appendData(term.data(), tlen);
            }
            appendData(&pcount, sizeof(pcount));
            for (const auto& p : postings) {
                uint64_t did = p.document_id;
                uint32_t freq = p.frequency;
                appendData(&did, sizeof(did));
                appendData(&freq, sizeof(freq));
            }
        });
    }

    // 3. Prepare Header
    IndexCacheHeader hdr;
    memcpy(hdr.magic, MAGIC_STR, 8);
    hdr.version = FORMAT_VERSION;
    hdr.doc_count = docs.size();
    hdr.term_count = termCount;
    hdr.avg_doc_length = index.getAvgDocLength();
    hdr.crc32 = computeCrc32(payload.data(), payload.size());

    // 4. Write to temp file
    HANDLE hFile = CreateFileW(tempPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD written = 0;
    if (!WriteFile(hFile, &hdr, sizeof(hdr), &written, NULL) || written != sizeof(hdr)) {
        CloseHandle(hFile);
        DeleteFileW(tempPath.c_str());
        return false;
    }

    if (!payload.empty()) {
        if (!WriteFile(hFile, payload.data(), static_cast<DWORD>(payload.size()), &written, NULL) ||
            written != payload.size()) {
            CloseHandle(hFile);
            DeleteFileW(tempPath.c_str());
            return false;
        }
    }

    FlushFileBuffers(hFile);
    CloseHandle(hFile);

    // 5. Crash-safe atomic replacement
    if (!MoveFileExW(tempPath.c_str(), finalPath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        DeleteFileW(tempPath.c_str());
        return false;
    }

    // Record in manifest
    uint64_t totalCacheBytes = sizeof(hdr) + payload.size();
    CacheManifest::recordFile(cacheDir, L"index.cache", totalCacheBytes);
    return true;
}

bool BinaryIndexCache::load(const std::wstring& cacheDir, CppInvertedIndex& index) {
    std::wstring cacheFile = cacheDir + L"\\index.cache";
    HANDLE hFile = CreateFileW(cacheFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    LARGE_INTEGER li;
    if (!GetFileSizeEx(hFile, &li) || li.QuadPart < static_cast<LONGLONG>(sizeof(IndexCacheHeader))) {
        CloseHandle(hFile);
        return false;
    }

    IndexCacheHeader hdr;
    DWORD readBytes = 0;
    if (!ReadFile(hFile, &hdr, sizeof(hdr), &readBytes, NULL) || readBytes != sizeof(hdr)) {
        CloseHandle(hFile);
        return false;
    }

    // Validate Magic Number & Version
    if (memcmp(hdr.magic, MAGIC_STR, 8) != 0 || hdr.version != FORMAT_VERSION) {
        CloseHandle(hFile);
        return false;
    }

    size_t payloadSize = static_cast<size_t>(li.QuadPart - sizeof(IndexCacheHeader));
    std::vector<uint8_t> payload(payloadSize);
    if (payloadSize > 0) {
        if (!ReadFile(hFile, payload.data(), static_cast<DWORD>(payloadSize), &readBytes, NULL) ||
            readBytes != payloadSize) {
            CloseHandle(hFile);
            return false;
        }
    }
    CloseHandle(hFile);

    // Validate Checksum (CRC32)
    uint32_t calcCrc = computeCrc32(payload.data(), payload.size());
    if (calcCrc != hdr.crc32) {
        std::cerr << "[WARN] index.cache CRC32 checksum mismatch (expected " << hdr.crc32
                  << ", calculated " << calcCrc << "). Discarding corrupt cache.\n";
        return false;
    }

    // Reset index to load cleanly
    index.getDocumentStore().clear();

    const uint8_t* p = payload.data();
    const uint8_t* end = p + payload.size();

    auto readData = [&](void* dest, size_t sz) -> bool {
        if (p + sz > end) return false;
        memcpy(dest, p, sz);
        p += sz;
        return true;
    };

    // Deserialize Document Records
    for (uint64_t i = 0; i < hdr.doc_count; i++) {
        uint64_t docId, fsize, mtime;
        uint32_t dlen, fmt, pathChars;

        if (!readData(&docId, sizeof(docId)) ||
            !readData(&fsize, sizeof(fsize)) ||
            !readData(&mtime, sizeof(mtime)) ||
            !readData(&dlen, sizeof(dlen)) ||
            !readData(&fmt, sizeof(fmt)) ||
            !readData(&pathChars, sizeof(pathChars))) {
            return false;
        }

        std::wstring path;
        if (pathChars > 0) {
            if (p + pathChars * sizeof(wchar_t) > end) return false;
            path.assign(reinterpret_cast<const wchar_t*>(p), pathChars);
            p += pathChars * sizeof(wchar_t);
        }

        DocMetadata meta;
        meta.id = docId;
        meta.fileSize = fsize;
        meta.lastWriteTime = mtime;
        meta.length = dlen;
        meta.format = static_cast<DocumentFormat>(fmt);
        meta.path = path;

        index.getDocumentStore().addExistingDocument(meta);
    }

    // Deserialize Term Postings
    for (uint64_t i = 0; i < hdr.term_count; i++) {
        uint16_t tlen;
        uint32_t pcount;

        if (!readData(&tlen, sizeof(tlen))) return false;
        std::string term;
        if (tlen > 0) {
            if (p + tlen > end) return false;
            term.assign(reinterpret_cast<const char*>(p), tlen);
            p += tlen;
        }

        if (!readData(&pcount, sizeof(pcount))) return false;
        for (uint32_t j = 0; j < pcount; j++) {
            uint64_t did;
            uint32_t freq;
            if (!readData(&did, sizeof(did)) || !readData(&freq, sizeof(freq))) return false;
            index.addTerm(term, did, freq);
        }
    }

    return true;
}

int BinaryIndexCache::updateIndexIncremental(const std::wstring& dirPath, const std::wstring& cacheDir, CppInvertedIndex& index) {
    // Attempt to load existing cache
    bool cacheLoaded = load(cacheDir, index);
    if (!cacheLoaded) {
        // Full initial index
        int res = index.indexDirectory(dirPath);
        if (res == 0) {
            save(cacheDir, index);
        }
        return res;
    }

    // Incremental walk
    // Compare files on disk against loaded DocumentStore
    std::unordered_map<std::wstring, bool> visited;

    struct IncContext {
        CppInvertedIndex* index;
        std::unordered_map<std::wstring, bool>* visited;
        size_t addedCount;
        size_t updatedCount;
        size_t unchangedCount;
    };

    IncContext ctx{&index, &visited, 0, 0, 0};

    auto incCallback = [](const Document* doc, void* udata) -> int {
        if (!doc || !doc->path) return 0;
        auto* c = static_cast<IncContext*>(udata);
        (*c->visited)[doc->path] = true;

        WIN32_FILE_ATTRIBUTE_DATA fad;
        uint64_t currentSize = 0;
        uint64_t currentMTime = 0;
        if (GetFileAttributesExW(doc->path, GetFileExInfoStandard, &fad)) {
            LARGE_INTEGER li;
            li.HighPart = fad.nFileSizeHigh;
            li.LowPart = fad.nFileSizeLow;
            currentSize = li.QuadPart;
            currentMTime = (static_cast<uint64_t>(fad.ftLastWriteTime.dwHighDateTime) << 32) |
                           fad.ftLastWriteTime.dwLowDateTime;
        }

        uint64_t existingDocId = c->index->getDocumentStore().findByPath(doc->path);
        if (existingDocId > 0) {
            const DocMetadata* meta = c->index->getDocumentStore().getMetadata(existingDocId);
            if (meta && meta->fileSize == currentSize && meta->lastWriteTime == currentMTime) {
                // File unchanged!
                c->unchangedCount++;
                return 0;
            }
            // File changed! Re-index
            c->updatedCount++;
            c->index->indexDocument(doc);
            c->index->getDocumentStore().setMetadata(existingDocId, currentSize, currentMTime, doc->format);
        } else {
            // New file! Index
            c->addedCount++;
            if (c->index->indexDocument(doc)) {
                uint64_t newId = c->index->getDocumentStore().findByPath(doc->path);
                if (newId > 0) {
                    c->index->getDocumentStore().setMetadata(newId, currentSize, currentMTime, doc->format);
                }
            }
        }
        return 0;
    };

    int walkRes = winfs_walk_directory(dirPath.c_str(), incCallback, &ctx);
    if (walkRes != 0) return walkRes;

    // Remove deleted files
    std::vector<uint64_t> toDelete;
    for (const auto& kv : index.getDocumentStore().getAllDocuments()) {
        if (visited.find(kv.second.path) == visited.end()) {
            toDelete.push_back(kv.first);
        }
    }
    for (uint64_t delId : toDelete) {
        index.getDocumentStore().removeDocument(delId);
    }

    // Persist updated index to disk
    save(cacheDir, index);
    return 0;
}

void CacheManifest::recordFile(const std::wstring& cacheDir, const std::wstring& filename, uint64_t size) {
    std::wstring manifestPath = cacheDir + L"\\manifest.txt";
    SYSTEMTIME st;
    GetSystemTime(&st);
    char timeStr[64];
    snprintf(timeStr, sizeof(timeStr), "%04d-%02d-%02d %02d:%02d:%02d",
             st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

    std::string line;
    std::string fnameA(filename.begin(), filename.end());
    line = fnameA + "|" + std::to_string(size) + "|" + timeStr + "\n";

    HANDLE hFile = CreateFileW(manifestPath.c_str(), FILE_APPEND_DATA, FILE_SHARE_READ, NULL,
                               OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written;
        WriteFile(hFile, line.c_str(), static_cast<DWORD>(line.size()), &written, NULL);
        CloseHandle(hFile);
    }
}

std::vector<std::pair<std::wstring, uint64_t>> CacheManifest::listTrackedFiles(const std::wstring& cacheDir) {
    std::vector<std::pair<std::wstring, uint64_t>> files;
    auto enumerate = [&](auto&& self, const std::wstring& directory,
                         const std::wstring& relative) -> void {
        WIN32_FIND_DATAW fd;
        HANDLE find = FindFirstFileW((directory + L"\\*").c_str(), &fd);
        if (find == INVALID_HANDLE_VALUE) return;
        do {
            if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0) continue;
            std::wstring relativePath = relative.empty() ? fd.cFileName : relative + L"\\" + fd.cFileName;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
                    self(self, directory + L"\\" + fd.cFileName, relativePath);
            } else {
                LARGE_INTEGER size;
                size.HighPart = fd.nFileSizeHigh;
                size.LowPart = fd.nFileSizeLow;
                files.emplace_back(relativePath, static_cast<uint64_t>(size.QuadPart));
            }
        } while (FindNextFileW(find, &fd));
        FindClose(find);
    };
    enumerate(enumerate, cacheDir, L"");
    return files;
}

bool CacheManifest::clean(const std::wstring& cacheDir, bool autoConfirm) {
    auto files = listTrackedFiles(cacheDir);
    if (files.empty()) {
        std::cout << "Cache directory is already empty: " << toUtf8(cacheDir) << "\n";
        return true;
    }

    uint64_t totalBytes = 0;
    std::cout << "Cache directory: " << toUtf8(cacheDir) << "\n";
    std::cout << "Files to delete:\n";
    for (const auto& f : files) {
        std::cout << "  - " << toUtf8(f.first) << " (" << f.second << " bytes)\n";
        totalBytes += f.second;
    }
    std::cout << "Total disk space to free: " << totalBytes << " bytes.\n";

    if (!autoConfirm) {
        std::cout << "Are you sure you want to delete these files? (y/N): ";
        char answer = 'n';
        std::cin >> answer;
        if (answer != 'y' && answer != 'Y') {
            std::cout << "Aborted. No files were deleted.\n";
            return false;
        }
    }

    size_t deletedCount = 0;
    for (const auto& f : files) {
        std::wstring fullPath = cacheDir + L"\\" + f.first;
        // Safety guard: ensure the file is strictly within cacheDir
        if (fullPath.find(cacheDir) == 0 && fullPath.find(L"..") == std::wstring::npos) {
            if (DeleteFileW(fullPath.c_str())) {
                deletedCount++;
            }
        }
    }

    std::cout << "Successfully cleaned " << deletedCount << " files (" << totalBytes << " bytes reclaimed).\n";
    return true;
}

} // namespace indexit
