#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <utility>

namespace indexit {

class CppInvertedIndex;

class BinaryIndexCache {
public:
    static uint32_t computeCrc32(const void* data, size_t length);
    static std::wstring getDefaultCacheDir();
    static bool ensureDirectoryExists(const std::wstring& dir);
    static bool checkDiskSpace(const std::wstring& cacheDir, uint64_t minBytesRequired = 50 * 1024 * 1024);
    static uint64_t getAvailableDiskSpace(const std::wstring& cacheDir);
    static uint64_t getCacheFileSize(const std::wstring& cacheDir);

    // Save and load
    static bool save(const std::wstring& cacheDir, const CppInvertedIndex& index);
    static bool load(const std::wstring& cacheDir, CppInvertedIndex& index);

    // Incremental index update
    static int updateIndexIncremental(const std::wstring& dirPath, const std::wstring& cacheDir, CppInvertedIndex& index);
};

class CacheManifest {
public:
    static void recordFile(const std::wstring& cacheDir, const std::wstring& filename, uint64_t size);
    static std::vector<std::pair<std::wstring, uint64_t>> listTrackedFiles(const std::wstring& cacheDir);
    static bool clean(const std::wstring& cacheDir, bool autoConfirm);
};

} // namespace indexit
