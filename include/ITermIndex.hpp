#pragma once
#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <functional>
#include "token.h"

extern "C" {
    typedef struct Trie Trie;
    typedef struct HashTable HashTable;
}

namespace indexit {

using TermVisitor = std::function<void(const std::string& term, const std::vector<Posting>& postings)>;

class ITermIndex {
public:
    virtual ~ITermIndex() = default;
    virtual void insert(const std::string& term, uint64_t docId, uint32_t frequency) = 0;
    virtual std::vector<Posting> lookup(const std::string& term) const = 0;
    virtual std::vector<Posting> lookupPrefix(const std::string& prefix) const { return {}; }
    virtual std::vector<std::pair<std::string, size_t>> getSuggestions(const std::string& prefix, size_t max_results = 10) const { return {}; }
    virtual void forEachTerm(TermVisitor visitor) const {}
    virtual size_t size() const { return 0; }
};

class TrieIndex : public ITermIndex {
private:
    ::Trie* trie_;
public:
    TrieIndex();
    ~TrieIndex() override;
    void insert(const std::string& term, uint64_t docId, uint32_t frequency) override;
    std::vector<Posting> lookup(const std::string& term) const override;
    std::vector<Posting> lookupPrefix(const std::string& prefix) const override;
    std::vector<std::pair<std::string, size_t>> getSuggestions(const std::string& prefix, size_t max_results = 10) const override;
};

class HashIndex : public ITermIndex {
private:
    ::HashTable* hash_;
public:
    HashIndex();
    ~HashIndex() override;
    void insert(const std::string& term, uint64_t docId, uint32_t frequency) override;
    std::vector<Posting> lookup(const std::string& term) const override;
    void forEachTerm(TermVisitor visitor) const override;
    size_t size() const override;
};

} // namespace indexit
