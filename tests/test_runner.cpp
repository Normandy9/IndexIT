#include <iostream>
#include <cassert>
#include "token.h"
#include "hashtable.h"
#include "trie.h"
#include "TopK.hpp"
#include "LruCache.hpp"

using namespace indexit;

void test_tokenizer() {
    Tokenizer* tz = tokenizer_create("Hello World! It's 2026.");
    Token t;
    assert(tokenizer_next(tz, &t) == 1);
    assert(std::string(t.text) == "hello");
    assert(tokenizer_next(tz, &t) == 1);
    assert(std::string(t.text) == "world");
    // "it" and "s" might be skipped if "it" is stopword
    // "2026"
    tokenizer_destroy(tz);
    std::cout << "Tokenizer tests passed.\n";
}

void test_hashtable() {
    HashTable* ht = hash_create();
    Posting p = {1, 5};
    hash_insert(ht, "test", p);
    Posting* res = hash_lookup(ht, "test");
    assert(res != nullptr);
    assert(res[0].document_id == 1);
    assert(res[0].frequency == 5);
    hash_destroy(ht);
    std::cout << "HashTable tests passed.\n";
}

void test_topk() {
    struct DummyScore {
        uint64_t id;
        double score;
        bool operator>(const DummyScore& other) const { return score > other.score; }
    };
    TopK<DummyScore, std::greater<DummyScore>> topk(2);
    topk.add({1, 10.0});
    topk.add({2, 5.0});
    topk.add({3, 15.0});
    auto res = topk.get();
    assert(res.size() == 2);
    assert(res[0].id == 3); // 15.0
    assert(res[1].id == 1); // 10.0
    std::cout << "TopK tests passed.\n";
}

int main() {
    test_tokenizer();
    test_hashtable();
    test_topk();
    std::cout << "All tests passed successfully.\n";
    return 0;
}
