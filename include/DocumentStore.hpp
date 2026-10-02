#pragma once
#include <string>
#include <unordered_map>
#include "document.h" // reuse C Document struct or redefine

namespace indexit {

class DocumentStore {
private:
    std::unordered_map<uint64_t, std::wstring> paths_;
    std::unordered_map<uint64_t, uint32_t> lengths_;
    uint64_t nextId_ = 1;

public:
    uint64_t addDocument(const std::wstring& path) {
        uint64_t id = nextId_++;
        paths_[id] = path;
        lengths_[id] = 0; // updated later
        return id;
    }

    void setLength(uint64_t docId, uint32_t length) {
        lengths_[docId] = length;
    }

    std::wstring getPath(uint64_t docId) const {
        auto it = paths_.find(docId);
        return it != paths_.end() ? it->second : L"";
    }

    uint32_t getLength(uint64_t docId) const {
        auto it = lengths_.find(docId);
        return it != lengths_.end() ? it->second : 0;
    }
    
    size_t size() const { return paths_.size(); }
    
    double getAvgLength() const {
        if (lengths_.empty()) return 0;
        double sum = 0;
        for (const auto& kv : lengths_) sum += kv.second;
        return sum / lengths_.size();
    }
};

} // namespace indexit
