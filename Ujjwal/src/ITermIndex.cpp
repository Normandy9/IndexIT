#include "ITermIndex.hpp"
#include <cstdlib>
extern "C" {
#include "trie.h"
#include "hashtable.h"
}

namespace indexit {

TrieIndex::TrieIndex() {
    trie_ = trie_create();
}

TrieIndex::~TrieIndex() {
    trie_destroy(trie_);
}

void TrieIndex::insert(const std::string& term, uint64_t docId, uint32_t frequency) {
    Posting p{docId, frequency};
    trie_insert(trie_, term.c_str(), p);
}

std::vector<Posting> TrieIndex::lookup(const std::string& term) const {
    size_t count = trie_search_count(trie_, term.c_str());
    Posting* p = trie_search(trie_, term.c_str());
    std::vector<Posting> res;
    if (p && count > 0) {
        res.assign(p, p + count);
    }
    return res;
}

std::vector<Posting> TrieIndex::lookupPrefix(const std::string& prefix) const {
    size_t count = 0;
    Posting* p = trie_prefix_search(trie_, prefix.c_str(), &count);
    std::vector<Posting> res;
    if (p && count > 0) {
        res.assign(p, p + count);
        free(p);
    }
    return res;
}

std::vector<std::pair<std::string, size_t>> TrieIndex::getSuggestions(const std::string& prefix, size_t max_results) const {
    std::vector<std::pair<std::string, size_t>> results;
    if (max_results == 0) return results;

    std::vector<TermSuggestion> buffer(max_results);
    size_t count = trie_autocomplete(trie_, prefix.c_str(), buffer.data(), max_results);
    for (size_t i = 0; i < count; i++) {
        results.emplace_back(buffer[i].term, buffer[i].doc_frequency);
    }
    return results;
}

HashIndex::HashIndex() {
    hash_ = hash_create();
}

HashIndex::~HashIndex() {
    hash_destroy(hash_);
}

void HashIndex::insert(const std::string& term, uint64_t docId, uint32_t frequency) {
    Posting p{docId, frequency};
    hash_insert(hash_, term.c_str(), p);
}

std::vector<Posting> HashIndex::lookup(const std::string& term) const {
    size_t count = hash_lookup_count(hash_, term.c_str());
    Posting* p = hash_lookup(hash_, term.c_str());
    std::vector<Posting> res;
    if (p && count > 0) {
        res.assign(p, p + count);
    }
    return res;
}

void HashIndex::forEachTerm(TermVisitor visitor) const {
    if (!visitor || !hash_) return;
    hash_for_each(hash_, [](const char* key, const Posting* postings, size_t count, void* udata) {
        auto* fn = static_cast<TermVisitor*>(udata);
        std::vector<Posting> vec;
        if (postings && count > 0) {
            vec.assign(postings, postings + count);
        }
        (*fn)(key, vec);
    }, &visitor);
}

size_t HashIndex::size() const {
    return hash_size(hash_);
}

} // namespace indexit
