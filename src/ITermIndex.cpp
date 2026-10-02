#include "ITermIndex.hpp"
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
    for (size_t i = 0; i < count; i++) {
        res.push_back(p[i]);
    }
    return res;
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
    for (size_t i = 0; i < count; i++) {
        res.push_back(p[i]);
    }
    return res;
}

} // namespace indexit
