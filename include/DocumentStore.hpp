#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include "document.h"

namespace indexit {

struct DocMetadata {
    uint64_t id;
    std::wstring path;
    uint64_t fileSize;
    uint64_t lastWriteTime;
    uint32_t length;
    DocumentFormat format;
};

class DocumentStore {
private:
    std::unordered_map<uint64_t, DocMetadata> docs_;
    std::unordered_map<std::wstring, uint64_t> pathToId_;
    uint64_t nextId_ = 1;

public:
    uint64_t addDocument(const std::wstring& path, uint64_t fileSize = 0, uint64_t lastWriteTime = 0, DocumentFormat format = FORMAT_UNKNOWN) {
        auto it = pathToId_.find(path);
        if (it != pathToId_.end()) {
            uint64_t existingId = it->second;
            docs_[existingId].fileSize = fileSize;
            docs_[existingId].lastWriteTime = lastWriteTime;
            docs_[existingId].format = format;
            return existingId;
        }

        uint64_t id = nextId_++;
        DocMetadata meta;
        meta.id = id;
        meta.path = path;
        meta.fileSize = fileSize;
        meta.lastWriteTime = lastWriteTime;
        meta.length = 0;
        meta.format = format;

        docs_[id] = meta;
        pathToId_[path] = id;
        return id;
    }

    void addExistingDocument(const DocMetadata& meta) {
        docs_[meta.id] = meta;
        pathToId_[meta.path] = meta.id;
        if (meta.id >= nextId_) {
            nextId_ = meta.id + 1;
        }
    }

    void setLength(uint64_t docId, uint32_t length) {
        auto it = docs_.find(docId);
        if (it != docs_.end()) {
            it->second.length = length;
        }
    }

    void setMetadata(uint64_t docId, uint64_t fileSize, uint64_t lastWriteTime, DocumentFormat format) {
        auto it = docs_.find(docId);
        if (it != docs_.end()) {
            it->second.fileSize = fileSize;
            it->second.lastWriteTime = lastWriteTime;
            it->second.format = format;
        }
    }

    std::wstring getPath(uint64_t docId) const {
        auto it = docs_.find(docId);
        return it != docs_.end() ? it->second.path : L"";
    }

    uint32_t getLength(uint64_t docId) const {
        auto it = docs_.find(docId);
        return it != docs_.end() ? it->second.length : 0;
    }

    const DocMetadata* getMetadata(uint64_t docId) const {
        auto it = docs_.find(docId);
        return it != docs_.end() ? &it->second : nullptr;
    }

    uint64_t findByPath(const std::wstring& path) const {
        auto it = pathToId_.find(path);
        return it != pathToId_.end() ? it->second : 0;
    }

    const std::unordered_map<uint64_t, DocMetadata>& getAllDocuments() const {
        return docs_;
    }

    void removeDocument(uint64_t docId) {
        auto it = docs_.find(docId);
        if (it != docs_.end()) {
            pathToId_.erase(it->second.path);
            docs_.erase(it);
        }
    }

    void clear() {
        docs_.clear();
        pathToId_.clear();
        nextId_ = 1;
    }
    
    size_t size() const { return docs_.size(); }
    
    double getAvgLength() const {
        if (docs_.empty()) return 0.0;
        double sum = 0.0;
        for (const auto& kv : docs_) sum += kv.second.length;
        return sum / docs_.size();
    }
};

} // namespace indexit
