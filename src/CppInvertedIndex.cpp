#include "CppInvertedIndex.hpp"
#include "token.h"
#include <sstream>
#include <algorithm>

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
    if (query.empty()) return {};

    // Tokenize query
    Tokenizer* tz = tokenizer_create(query.c_str());
    if (!tz) return {};

    std::vector<std::string> terms;
    Token t;
    while (tokenizer_next(tz, &t)) {
        terms.push_back(t.text);
    }
    tokenizer_destroy(tz);

    if (terms.empty()) return {};

    // For AND logic, intersect postings. 
    // Start with postings of the first term.
    std::vector<Posting> current_postings = exactIndex_->lookup(terms[0]);
    
    // Simple intersection for AND-style
    for (size_t i = 1; i < terms.size(); ++i) {
        std::vector<Posting> next_postings = exactIndex_->lookup(terms[i]);
        std::vector<Posting> intersection;
        
        // Both are unsorted or sorted? The hash table insertion is unsorted/append based on appearance.
        // It's safer to sort them by document_id to intersect quickly, or just use a nested loop since N is small.
        // Let's sort both by document_id for linear intersection.
        std::sort(current_postings.begin(), current_postings.end(), [](const Posting& a, const Posting& b) {
            return a.document_id < b.document_id;
        });
        std::sort(next_postings.begin(), next_postings.end(), [](const Posting& a, const Posting& b) {
            return a.document_id < b.document_id;
        });

        size_t a = 0, b_idx = 0;
        while (a < current_postings.size() && b_idx < next_postings.size()) {
            if (current_postings[a].document_id == next_postings[b_idx].document_id) {
                // Combine frequency or handle it differently?
                // For scoring, we'll just keep the first posting to represent presence, but score should actually sum over terms.
                // It's better to calculate score per term per document later.
                // So intersection just needs the document ID. Let's store document ID and combined score.
                intersection.push_back(current_postings[a]);
                a++;
                b_idx++;
            } else if (current_postings[a].document_id < next_postings[b_idx].document_id) {
                a++;
            } else {
                b_idx++;
            }
        }
        current_postings = intersection;
        if (current_postings.empty()) break;
    }

    // Now current_postings contains documents that have ALL terms.
    TopK<ScoredDocument, ScoreCompare> heap(k);
    
    for (const auto& p : current_postings) {
        double total_score = 0;
        for (const auto& term : terms) {
            // Find frequency of this term in this document
            std::vector<Posting> tp = exactIndex_->lookup(term);
            uint32_t tf = 0;
            for (const auto& doc_p : tp) {
                if (doc_p.document_id == p.document_id) {
                    tf = doc_p.frequency;
                    break;
                }
            }
            total_score += ranker.score(term, tf, p.document_id, *this);
        }
        heap.add({p.document_id, total_score});
    }
    
    return heap.get();
}

} // namespace indexit
