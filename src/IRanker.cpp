#include "IRanker.hpp"
#include <cmath>

namespace indexit {

double TfIdfRanker::score(const std::string& term, uint32_t termFreqInDoc, uint64_t docId, const IndexStats& stats) {
    double tf = 1.0 + std::log(termFreqInDoc);
    uint32_t df = stats.docFrequency(term);
    double idf = std::log(static_cast<double>(stats.getTotalDocs()) / (1.0 + df));
    return tf * idf;
}

double Bm25Ranker::score(const std::string& term, uint32_t termFreqInDoc, uint64_t docId, const IndexStats& stats) {
    uint32_t df = stats.docFrequency(term);
    double idf = std::log((stats.getTotalDocs() - df + 0.5) / (df + 0.5) + 1.0);
    double tf = termFreqInDoc;
    double dl = stats.docLength(docId);
    double avgdl = stats.getAvgDocLength() > 0 ? stats.getAvgDocLength() : 1.0;
    
    double num = tf * (k1 + 1.0);
    double den = tf + k1 * (1.0 - b + b * (dl / avgdl));
    return idf * (num / den);
}

} // namespace indexit
