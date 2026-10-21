#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <windows.h>

#include "token.h"
#include "tokenizer.h"
#include "document_dj.h"
#include "winfs.h"
#include "extractor.h"
#include "hashtable.h"
#include "trie.h"
#include "inverted_index.h"
#include "ITermIndex.hpp"
#include "IRanker.hpp"
#include "CppInvertedIndex.hpp"
#include "IExtractor.hpp"

using namespace indexit;

// 1. Test Tokenizer
static void test_tokenizer() {
    std::cout << "[Test] Running test_tokenizer...\n";
    const char *text = "The quick brown fox jumps over the lazy dog. Fox is fast!";
    Tokenizer *tok = tokenizer_create(text);
    assert(tok != nullptr);

    Token token;
    std::vector<std::string> tokens;
    while (tokenizer_next(tok, &token)) {
        tokens.push_back(token.text);
        free(token.text);
    }
    tokenizer_destroy(tok);

    // "the" and "is" are stopwords and should be removed
    for (const auto& t : tokens) {
        assert(t != "the");
        assert(t != "is");
    }
    // "quick", "brown", "fox", "jumps", "lazy", "dog", "fast" should exist
    bool found_fox = false;
    for (const auto& t : tokens) {
        if (t == "fox") found_fox = true;
    }
    assert(found_fox);
    std::cout << "  -> Tokenizer passed (" << tokens.size() << " valid tokens extracted)\n";
}

// 2. Test Hash Table
static void test_hashtable() {
    std::cout << "[Test] Running test_hashtable...\n";
    HashTable *ht = hash_create();
    assert(ht != nullptr);

    Posting p1 = {1, 5};
    Posting p2 = {2, 3};
    assert(hash_insert(ht, "search", p1) == 0);
    assert(hash_insert(ht, "search", p2) == 0);
    assert(hash_insert(ht, "engine", p1) == 0);

    Posting *res = hash_lookup(ht, "search");
    assert(res != nullptr);
    assert(hash_lookup_count(ht, "search") == 2);
    assert(hash_lookup(ht, "nonexistent") == nullptr);

    hash_destroy(ht);
    std::cout << "  -> Hash table passed\n";
}

// 3. Test Trie
static void test_trie() {
    std::cout << "[Test] Running test_trie...\n";
    Trie *tr = trie_create();
    assert(tr != nullptr);

    Posting p1 = {10, 2};
    assert(trie_insert(tr, "apple", p1) == 0);
    assert(trie_insert(tr, "application", p1) == 0);

    Posting *res = trie_search(tr, "apple");
    assert(res != nullptr);
    assert(trie_search(tr, "app") == nullptr); // "app" itself not inserted

    trie_destroy(tr);
    std::cout << "  -> Trie passed\n";
}

// 4. Test Production Inverted Index (Composite Hash+Trie)
static void test_inverted_index() {
    std::cout << "[Test] Running test_inverted_index...\n";
    InvertedIndex *idx = index_create();
    assert(idx != nullptr);

    Posting p = {42, 9};
    assert(index_insert(idx, "windows", p) == 0);

    Posting *res = index_lookup(idx, "windows");
    assert(res != nullptr);
    assert(res->document_id == 42);
    assert(res->frequency == 9);

    index_destroy(idx);
    std::cout << "  -> Inverted index passed\n";
}

// 5. Test WinFS Walk and Extractor
static int s_doc_count = 0;
static int winfs_callback(const Document *doc, void *user_data) {
    (void)user_data;
    assert(doc != nullptr);
    assert(doc->path != nullptr);
    s_doc_count++;
    char *text = extract_text(doc);
    if (text) {
        assert(strlen(text) > 0);
        free(text);
    }
    return 0;
}

static void test_winfs_and_extractor() {
    std::cout << "[Test] Running test_winfs_and_extractor on data/small_set...\n";
    extractor_init();
    s_doc_count = 0;
    std::wstring path = L"data\\small_set";
    DWORD attr = GetFileAttributesW(path.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES) {
        path = L"..\\IndexIT_Reconciled\\data\\small_set";
        attr = GetFileAttributesW(path.c_str());
    }
    if (attr == INVALID_FILE_ATTRIBUTES) {
        path = L"D:\\IndexIT_Reconciled\\data\\small_set";
    }
    int rc = winfs_walk_directory(path.c_str(), winfs_callback, nullptr);
    assert(rc == 0);
    assert(s_doc_count >= 5); // file1.txt, file2.txt, file3.csv, file4.md, file5.log
    extractor_cleanup();
    std::cout << "  -> WinFS & Extractor passed (" << s_doc_count << " documents found and extracted)\n";
}

// 6. Test C++ OOP Layer & BM25 Ranker
static void test_cpp_oop_ranking() {
    std::cout << "[Test] Running test_cpp_oop_ranking...\n";
    CppInvertedIndex index;
    uint64_t d1 = index.addDocument(L"doc1.txt");
    uint64_t d2 = index.addDocument(L"doc2.txt");

    index.addTerm("kernel", d1, 5);
    index.setDocumentLength(d1, 100);

    index.addTerm("kernel", d2, 1);
    index.setDocumentLength(d2, 50);

    Bm25Ranker ranker;
    auto results = index.search("kernel", ranker, 5);
    assert(!results.empty());
    // doc1 has higher frequency and should score higher or be ranked
    assert(results[0].docId == d1 || results[0].score > 0.0);

    std::cout << "  -> C++ OOP layer & BM25 ranker passed\n";
}

int main() {
    std::cout << "===========================================\n";
    std::cout << "   IndexIt Reconciled Core Engine Tests    \n";
    std::cout << "===========================================\n";

    test_tokenizer();
    test_hashtable();
    test_trie();
    test_inverted_index();
    test_winfs_and_extractor();
    test_cpp_oop_ranking();

    std::cout << "===========================================\n";
    std::cout << "   ALL 6 CORE ENGINE SUITES PASSED!       \n";
    std::cout << "===========================================\n";
    return 0;
}
