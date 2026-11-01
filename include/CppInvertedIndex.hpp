#pragma once
#include "ITermIndex.hpp"
#include "IRanker.hpp"
#include "DocumentStore.hpp"
#include "TopK.hpp"
#include "document.h"
#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <functional>

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
    uint32_t docFrequency(const std::string& term) const override;

    // Core C++ indexing methods (used by both CLI and UI)
    bool indexDocument(const Document* doc);
    int indexDirectory(const std::wstring& dirPath, std::function<void(const std::wstring&, uint64_t)> progressCb = nullptr);

    // Search (supports exact terms and prefix queries with 'term*' syntax)
    std::vector<ScoredDocument> search(const std::string& query, IRanker& ranker, size_t k = 10);

    // Autocomplete suggestions ranked by document frequency
    std::vector<std::pair<std::string, size_t>> suggest(const std::string& prefix, size_t max_results = 10) const;
    std::vector<std::pair<std::string, size_t>> suggestSimilar(const std::string& term, size_t max_results = 5) const;

    std::wstring getPath(uint64_t docId) const { return docStore_.getPath(docId); }

    ITermIndex* getExactIndex() const { return exactIndex_.get(); }
    ITermIndex* getPrefixIndex() const { return prefixIndex_.get(); }
    const DocumentStore& getDocumentStore() const { return docStore_; }
    DocumentStore& getDocumentStore() { return docStore_; }
};

} // namespace indexit
