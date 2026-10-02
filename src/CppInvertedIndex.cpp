#include "CppInvertedIndex.hpp"
#include <sstream>

namespace indexit {

CppInvertedIndex::CppInvertedIndex() {
    exactIndex_ = std::make_unique<HashIndex>();
    prefixIndex_ = std::make_unique<TrieIndex>();
}

void CppInvertedIndex::addTerm(const std::string& term, uint64_t docId, uint32_t frequency) {
    exactIndex_->insert(term, docId, frequency);
    prefixIndex_->insert(term, docId, frequency);
}

uint64_t CppInvertedIndex::addDocument(const std::wstring& path) {
    return docStore_.addDocument(path);
}

void CppInvertedIndex::setDocumentLength(uint64_t docId, uint32_t length) {
    docStore_.setLength(docId, length);
}

struct ScoreCompare {
    bool operator()(const ScoredDocument& a, const ScoredDocument& b) const {
        return a.score > b.score; // min-heap based on score
    }
};

std::vector<ScoredDocument> CppInvertedIndex::search(const std::string& query, IRanker& ranker, size_t k) {
    // Convert query to lowercase to match the tokenizer
    std::string lowerQuery = query;
    for (char& c : lowerQuery) {
        c = std::tolower(static_cast<unsigned char>(c));
    }

    std::vector<Posting> postings = exactIndex_->lookup(lowerQuery);
    TopK<ScoredDocument, ScoreCompare> heap(k);
    
    for (const auto& p : postings) {
        double score = ranker.score(lowerQuery, p.frequency, p.document_id, *this);
        heap.add({p.document_id, score});
    }
    
    return heap.get();
}

} // namespace indexit
