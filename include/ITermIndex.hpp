#pragma once
#include <string>
#include <vector>
#include "token.h" // reuse the C struct or redefine
#include <memory>
extern "C" {
    typedef struct Trie Trie;
    typedef struct HashTable HashTable;
}

namespace indexit {

class ITermIndex {
public:
    virtual ~ITermIndex() = default;
    virtual void insert(const std::string& term, uint64_t docId, uint32_t frequency) = 0;
    virtual std::vector<Posting> lookup(const std::string& term) const = 0;
};

// C++ wrappers over the C implementations
class TrieIndex : public ITermIndex {
    ::Trie* trie_;
public:
    TrieIndex();
    ~TrieIndex() override;
    void insert(const std::string& term, uint64_t docId, uint32_t frequency) override;
    std::vector<Posting> lookup(const std::string& term) const override;
};

class HashIndex : public ITermIndex {
    ::HashTable* hash_;
public:
    HashIndex();
    ~HashIndex() override;
    void insert(const std::string& term, uint64_t docId, uint32_t frequency) override;
    std::vector<Posting> lookup(const std::string& term) const override;
};

} // namespace indexit
