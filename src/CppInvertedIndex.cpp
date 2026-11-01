#include "CppInvertedIndex.hpp"
#include "IExtractor.hpp"
#include "token.h"
#include "SearchSnippet.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <windows.h>

extern "C" {
#include "winfs.h"
}

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

uint32_t CppInvertedIndex::docFrequency(const std::string& term) const {
    if (!term.empty() && term.back() == '*') {
        std::string p = term.substr(0, term.size() - 1);
        return static_cast<uint32_t>(prefixIndex_->lookupPrefix(p).size());
    }
    return static_cast<uint32_t>(exactIndex_->lookup(term).size());
}

bool CppInvertedIndex::indexDocument(const Document* doc) {
    if (!doc || !doc->path || doc->format == FORMAT_UNKNOWN) return false;

    auto extractor = ExtractorFactory::create(doc->format);
    if (!extractor) return false;

    std::string text = extractor->extract(doc->path);
    if (text.empty()) return false;

    uint64_t docId = docStore_.addDocument(doc->path, doc->size, static_cast<uint64_t>(doc->timestamp), doc->format);
    Tokenizer* tok = tokenizer_create(text.c_str());
    uint32_t len = 0;
    if (tok) {
        Token token;
        while (tokenizer_next(tok, &token)) {
            addTerm(token.text, docId, 1);
            free(token.text);
            len++;
        }
        tokenizer_destroy(tok);
    }
    setDocumentLength(docId, len);
    return true;
}

struct IndexCallbackContext {
    CppInvertedIndex* index;
    std::function<void(const std::wstring&, uint64_t)> progressCb;
};

static int shared_index_callback(const Document* doc, void* user_data) {
    if (!doc || !user_data) return 0;
    auto* ctx = static_cast<IndexCallbackContext*>(user_data);
    if (ctx->index->indexDocument(doc)) {
        if (ctx->progressCb) {
            ctx->progressCb(doc->path, ctx->index->getTotalDocs());
        }
    }
    return 0;
}

int CppInvertedIndex::indexDirectory(const std::wstring& dirPath, std::function<void(const std::wstring&, uint64_t)> progressCb) {
    DWORD attr = GetFileAttributesW(dirPath.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
        return -1;
    }
    IndexCallbackContext ctx{this, progressCb};
    return winfs_walk_directory(dirPath.c_str(), shared_index_callback, &ctx);
}


std::string CppInvertedIndex::getSnippet(uint64_t docId, const std::string& query, size_t context) const {
    const DocMetadata* metadata = docStore_.getMetadata(docId);
    if (!metadata) return {};
    std::unique_ptr<IExtractor> extractor = ExtractorFactory::create(metadata->format);
    if (!extractor) return {};
    const std::string extracted = extractor->extract(metadata->path);
    return makeSearchSnippet(extracted, query, context);
}

struct ScoreCompare {
    bool operator()(const ScoredDocument& a, const ScoredDocument& b) const {
        return a.score > b.score; // min-heap based on score
    }
};

struct ParsedQueryTerm {
    std::string rawTerm;
    std::string cleanTerm;
    bool isPrefix;
};

static std::vector<ParsedQueryTerm> parse_query(const std::string& query) {
    std::vector<ParsedQueryTerm> result;
    std::istringstream iss(query);
    std::string word;

    while (iss >> word) {
        // Strip punctuation except trailing '*'
        bool isPrefix = false;
        if (word.size() > 1 && word.back() == '*') {
            isPrefix = true;
            word.pop_back();
        }

        std::string clean;
        for (char c : word) {
            if (isalnum(static_cast<unsigned char>(c))) {
                clean += static_cast<char>(tolower(static_cast<unsigned char>(c)));
            }
        }

        if (!clean.empty()) {
            ParsedQueryTerm qt;
            qt.isPrefix = isPrefix;
            qt.cleanTerm = clean;
            qt.rawTerm = isPrefix ? (clean + "*") : clean;
            result.push_back(qt);
        }
    }
    return result;
}

std::vector<ScoredDocument> CppInvertedIndex::search(const std::string& query, IRanker& ranker, size_t k) {
    if (query.empty()) return {};

    std::vector<ParsedQueryTerm> terms = parse_query(query);
    if (terms.empty()) return {};

    // Get postings for first term
    std::vector<Posting> current_postings;
    if (terms[0].isPrefix) {
        current_postings = prefixIndex_->lookupPrefix(terms[0].cleanTerm);
    } else {
        current_postings = exactIndex_->lookup(terms[0].cleanTerm);
    }

    if (current_postings.empty()) return {};

    // Two-pointer linear merge intersection for multi-term AND query
    for (size_t i = 1; i < terms.size(); ++i) {
        std::vector<Posting> next_postings;
        if (terms[i].isPrefix) {
            next_postings = prefixIndex_->lookupPrefix(terms[i].cleanTerm);
        } else {
            next_postings = exactIndex_->lookup(terms[i].cleanTerm);
        }
        if (next_postings.empty()) return {};

        std::vector<Posting> intersection;
        intersection.reserve(std::min(current_postings.size(), next_postings.size()));

        size_t a = 0, b = 0;
        while (a < current_postings.size() && b < next_postings.size()) {
            if (current_postings[a].document_id == next_postings[b].document_id) {
                intersection.push_back(current_postings[a]);
                a++;
                b++;
            } else if (current_postings[a].document_id < next_postings[b].document_id) {
                a++;
            } else {
                b++;
            }
        }
        current_postings = std::move(intersection);
        if (current_postings.empty()) return {};
    }

    // Rank matching documents using IRanker and TopK
    TopK<ScoredDocument, ScoreCompare> heap(k);
    for (const auto& p : current_postings) {
        double total_score = 0.0;
        for (const auto& term : terms) {
            std::vector<Posting> tp;
            if (term.isPrefix) {
                tp = prefixIndex_->lookupPrefix(term.cleanTerm);
            } else {
                tp = exactIndex_->lookup(term.cleanTerm);
            }

            uint32_t tf = 0;
            // Binary search in sorted tp for p.document_id
            size_t low = 0, high = tp.size();
            while (low < high) {
                size_t mid = low + (high - low) / 2;
                if (tp[mid].document_id == p.document_id) {
                    tf = tp[mid].frequency;
                    break;
                } else if (tp[mid].document_id < p.document_id) {
                    low = mid + 1;
                } else {
                    high = mid;
                }
            }
            if (tf > 0) {
                total_score += ranker.score(term.rawTerm, tf, p.document_id, *this);
            }
        }
        heap.add({p.document_id, total_score});
    }

    return heap.get();
}

std::vector<std::pair<std::string, size_t>> CppInvertedIndex::suggest(const std::string& prefix, size_t max_results) const {
    if (!prefixIndex_) return {};
    return prefixIndex_->getSuggestions(prefix, max_results);
}

std::vector<std::pair<std::string, size_t>> CppInvertedIndex::suggestSimilar(const std::string& term, size_t max_results) const {
    struct Candidate { std::string word; size_t df; unsigned distance; };
    std::vector<Candidate> candidates;
    if (term.empty() || !exactIndex_) return {};
    exactIndex_->forEachTerm([&](const std::string& word, const std::vector<Posting>& postings) {
        if (word == term || (word.size() > term.size()+2) || (term.size() > word.size()+2)) return;
        std::vector<unsigned> prev(term.size()+1), cur(term.size()+1);
        for(size_t i=0;i<=term.size();++i) prev[i]=static_cast<unsigned>(i);
        for(size_t i=1;i<=word.size();++i) {
            cur[0]=static_cast<unsigned>(i); unsigned rowMin=cur[0];
            for(size_t j=1;j<=term.size();++j) {
                unsigned cost=word[i-1]==term[j-1]?0:1;
                cur[j]=std::min(std::min(cur[j-1]+1,prev[j]+1),prev[j-1]+cost);
                rowMin=std::min(rowMin,cur[j]);
            }
            if(rowMin>2)return;
            prev.swap(cur);
        }
        unsigned d=prev[term.size()];
        if(d<=2)candidates.push_back(Candidate{word,postings.size(),d});
    });
    std::sort(candidates.begin(),candidates.end(),[](const Candidate& a,const Candidate& b){return a.distance!=b.distance?a.distance<b.distance:(a.df!=b.df?a.df>b.df:a.word<b.word);});
    std::vector<std::pair<std::string,size_t>> out;
    for(size_t i=0;i<candidates.size()&&i<max_results;++i)out.push_back(std::make_pair(candidates[i].word,candidates[i].df));
    return out;
}

} // namespace indexit