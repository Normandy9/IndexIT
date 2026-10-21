#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace indexit {

struct ScoredDocument {
    uint64_t docId;
    double score;
};

// Required stats for TF-IDF / BM25
class IndexStats {
public:
    virtual ~IndexStats() = default;
    virtual uint64_t getTotalDocs() const = 0;
    virtual double getAvgDocLength() const = 0;
    virtual uint32_t docLength(uint64_t docId) const = 0;
    virtual uint32_t docFrequency(const std::string& term) const = 0;
};

class IRanker {
public:
    virtual ~IRanker() = default;
    virtual double score(const std::string& term, uint32_t termFreqInDoc, uint64_t docId, const IndexStats& stats) = 0;
};

class TfIdfRanker : public IRanker {
public:
    double score(const std::string& term, uint32_t termFreqInDoc, uint64_t docId, const IndexStats& stats) override;
};

class Bm25Ranker : public IRanker {
private:
    double k1 = 1.5;
    double b = 0.75;
public:
    double score(const std::string& term, uint32_t termFreqInDoc, uint64_t docId, const IndexStats& stats) override;
};

} // namespace indexit
