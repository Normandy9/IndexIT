#pragma once
#include "ITermIndex.hpp"
#include "IRanker.hpp"
#include "DocumentStore.hpp"
#include "TopK.hpp"
#include <memory>

namespace indexit {

class CppInvertedIndex : public IndexStats {
private:
    std::unique_ptr<ITermIndex> exactIndex_;
    std::unique_ptr<ITermIndex> prefixIndex_;
    DocumentStore docStore_;

public:
    CppInvertedIndex();

    void addTerm(const std::string& term, uint64_t docId, uint32_t frequency);
    uint64_t addDocument(const std::wstring& path);
    void setDocumentLength(uint64_t docId, uint32_t length);

    // IndexStats interface
    uint64_t getTotalDocs() const override { return docStore_.size(); }
    double getAvgDocLength() const override { return docStore_.getAvgLength(); }
    uint32_t docLength(uint64_t docId) const override { return docStore_.getLength(docId); }
    uint32_t docFrequency(const std::string& term) const override {
        return exactIndex_->lookup(term).size();
    }
    
    // Search
    std::vector<ScoredDocument> search(const std::string& query, IRanker& ranker, size_t k = 10);
    std::wstring getPath(uint64_t docId) const { return docStore_.getPath(docId); }
};

} // namespace indexit
