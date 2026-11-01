#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "RAII.hpp"
#include "DocumentStore.hpp"

namespace indexit {

struct SearchHistoryRecord {
    int id;
    std::string query;
    std::string timestamp;
    int resultCount;
};

class DatabaseManager {
private:
    SqliteConnection conn_;
public:
    explicit DatabaseManager(const std::string& dbPath);
    void initSchema();
    void saveDocuments(const DocumentStore& store);
    void loadDocuments(DocumentStore& store);
    void recordManifest(const std::string& path, uint64_t size, const std::string& created);
    void recordSearchHistory(const std::string& query, int resultCount);
    std::vector<SearchHistoryRecord> getSearchHistory(int limit = 50);
};

} // namespace indexit
