#include <iostream>
#include <cassert>
#include <cmath>
#include <string>
#include <vector>
#include <windows.h>
#include "token.h"
#include "hashtable.h"
#include "trie.h"
#include "TopK.hpp"
#include "LruCache.hpp"
#include "CacheManager.hpp"
#include "RAII.hpp"
#include "CppInvertedIndex.hpp"
#include "IRanker.hpp"
#include "BinaryIndexCache.hpp"
#include "db.h"
#include "IExtractor.hpp"

using namespace indexit;

void test_extractors() {
    struct Fixture { const wchar_t* file; DocumentFormat format; const char* phrase; };
    const Fixture cases[] = {
        {L"sample.docx", FORMAT_DOCX, "Docx Extraction Test Phrase"},
        {L"sample.pptx", FORMAT_PPTX, "Presentation Slide Test Phrase"},
        {L"sample.xlsx", FORMAT_XLSX, "Spreadsheet Cell Test Phrase"},
        {L"sample.odt", FORMAT_ODT, "OpenDocument Text Test Phrase"},
        {L"sample.zip", FORMAT_ZIP, "Archive Compressed Test Phrase"},
        {L"sample.rtf", FORMAT_RTF, "RichText Document Test Phrase"},
        {L"sample.eml", FORMAT_EML, "Email Message Test Phrase"},
        {L"sample.html", FORMAT_HTML, "HyperText Markup Test Phrase"},
        {L"sample.pdf", FORMAT_PDF, "Portable Document Test Phrase"}
    };
    for (size_t i=0;i<sizeof(cases)/sizeof(cases[0]);++i) {
        std::wstring path=L"";
        std::string base=INDEXIT_SOURCE_DIR;
        for(size_t j=0;j<base.size();++j) path.push_back(static_cast<unsigned char>(base[j]));
        path += L"\\data\\test_fixtures\\"; path += cases[i].file;
        std::unique_ptr<IExtractor> ex=ExtractorFactory::create(cases[i].format);
        assert(ex.get()!=nullptr);
        std::string text=ex->extract(path);
        assert(text.find(cases[i].phrase)!=std::string::npos);
    }
    std::cout << "Extractor fixtures (DOCX/PPTX/XLSX/ODT/ZIP/RTF/EML/HTML/PDF) passed.\n";
}

void test_fuzzy_matching() {
    CppInvertedIndex index;
    index.addTerm("computer", 1, 1);
    index.addTerm("compile", 2, 1);
    index.addTerm("composure", 3, 1);
    std::vector<std::pair<std::string,size_t>> hits=index.suggestSimilar("computre",5);
    assert(!hits.empty() && hits[0].first=="computer");
    assert(index.suggestSimilar("zzzzzzzz",5).empty());
    std::cout << "Fuzzy matching distance bound tests passed.\n";
}

void test_tokenizer() {
    Tokenizer* tz = tokenizer_create("Hello World! It's 2026.");
    Token t;
    assert(tokenizer_next(tz, &t) == 1);
    assert(std::string(t.text) == "hello");
    free(t.text);
    assert(tokenizer_next(tz, &t) == 1);
    assert(std::string(t.text) == "world");
    free(t.text);
    tokenizer_destroy(tz);
    tz = tokenizer_create("CAF\xC3\x89 au lait \xE4\xB8\x96\xE7\x95\x8C");
    assert(tokenizer_next(tz, &t) == 1);
    assert(std::string(t.text) == "caf\xC3\x89"); free(t.text);
    assert(tokenizer_next(tz, &t) == 1);
    assert(std::string(t.text) == "au"); free(t.text);
    assert(tokenizer_next(tz, &t) == 1);
    assert(std::string(t.text) == "lait"); free(t.text);
    assert(tokenizer_next(tz, &t) == 1);
    assert(std::string(t.text) == "\xE4\xB8\x96\xE7\x95\x8C"); free(t.text);
    tokenizer_destroy(tz);
    std::cout << "Tokenizer tests passed.\n";
}

void test_hashtable() {
    HashTable* ht = hash_create_with_capacity(4);
    assert(hash_num_buckets(ht) == 4);
    assert(hash_size(ht) == 0);

    hash_insert(ht, "apple", {5, 2});
    hash_insert(ht, "apple", {2, 1});
    hash_insert(ht, "apple", {8, 3});
    hash_insert(ht, "apple", {2, 4});

    size_t count = hash_lookup_count(ht, "apple");
    assert(count == 3);
    Posting* p = hash_lookup(ht, "apple");
    assert(p != nullptr);
    assert(p[0].document_id == 2 && p[0].frequency == 5);
    assert(p[1].document_id == 5 && p[1].frequency == 2);
    assert(p[2].document_id == 8 && p[2].frequency == 3);

    hash_insert(ht, "banana", {1, 1});
    hash_insert(ht, "cherry", {1, 1});
    hash_insert(ht, "date", {1, 1});
    hash_insert(ht, "elderberry", {1, 1});

    assert(hash_num_buckets(ht) > 4);
    assert(hash_size(ht) == 5);

    Posting* pb = hash_lookup(ht, "banana");
    assert(pb != nullptr && pb[0].document_id == 1);

    hash_destroy(ht);
    std::cout << "HashTable tests passed (resizing & sorted postings verified).\n";
}

void test_trie_prefix_and_autocomplete() {
    Trie* tr = trie_create();

    trie_insert(tr, "compute", {1, 1});
    trie_insert(tr, "compute", {2, 1});
    trie_insert(tr, "compute", {3, 1});

    trie_insert(tr, "computer", {2, 1});
    trie_insert(tr, "computer", {4, 1});

    trie_insert(tr, "compile", {5, 1});
    trie_insert(tr, "different", {1, 1});

    TermSuggestion sugs[10];
    size_t num_sugs = trie_autocomplete(tr, "comp", sugs, 10);
    assert(num_sugs == 3);
    assert(std::string(sugs[0].term) == "compute" && sugs[0].doc_frequency == 3);
    assert(std::string(sugs[1].term) == "computer" && sugs[1].doc_frequency == 2);
    assert(std::string(sugs[2].term) == "compile" && sugs[2].doc_frequency == 1);

    size_t p_count = 0;
    Posting* p_res = trie_prefix_search(tr, "comp", &p_count);
    assert(p_res != nullptr);
    assert(p_count == 5);
    for (size_t i = 1; i < p_count; i++) {
        assert(p_res[i - 1].document_id < p_res[i].document_id);
    }
    assert(p_res[1].document_id == 2 && p_res[1].frequency == 2);
    free(p_res);

    size_t no_count = 0;
    Posting* no_res = trie_prefix_search(tr, "xyz", &no_count);
    assert(no_res == nullptr && no_count == 0);

    size_t no_sug_count = trie_autocomplete(tr, "xyz", sugs, 10);
    assert(no_sug_count == 0);

    trie_destroy(tr);
    std::cout << "Trie prefix hits, no-match, and autocomplete tests passed.\n";
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
    assert(res[0].id == 3);
    assert(res[1].id == 1);
    std::cout << "TopK tests passed.\n";
}

void test_cache_manager_byte_cap() {
    CacheManager cm(10, 50);

    cm.put("k1", "012345678901234567");
    assert(cm.size() == 1);
    assert(cm.currentBytes() == 20);

    cm.put("k2", "abcdefghijklmnopqr");
    assert(cm.size() == 2);
    assert(cm.currentBytes() == 40);

    cm.put("k3", "ABCDEFGHIJKLMNOPQR");
    assert(cm.size() == 2);
    assert(cm.currentBytes() == 40);

    std::string val;
    assert(!cm.get("k1", val));
    assert(cm.get("k2", val));
    assert(cm.get("k3", val));

    cm.invalidate("k2");
    assert(!cm.get("k2", val));
    assert(cm.size() == 1);
    assert(cm.currentBytes() == 20);

    std::cout << "CacheManager hard byte cap tests passed.\n";
}

void test_scoped_timer() {
    ScopedTimer timer("test");
    Sleep(10);
    assert(timer.elapsedMs() >= 5);
    assert(timer.elapsedUs() > 0.0);
    assert(timer.elapsedSec() >= 0.0);
    std::cout << "ScopedTimer tests passed.\n";
}

static std::wstring resolve_dataset_path() {
    std::wstring p = L"data/small_set";
    if (GetFileAttributesW(p.c_str()) != INVALID_FILE_ATTRIBUTES) return p;
    p = L"../data/small_set";
    if (GetFileAttributesW(p.c_str()) != INVALID_FILE_ATTRIBUTES) return p;
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    wchar_t* lastSlash = wcsrchr(exePath, L'\\');
    if (lastSlash) *lastSlash = 0;
    p = std::wstring(exePath) + L"/../data/small_set";
    if (GetFileAttributesW(p.c_str()) != INVALID_FILE_ATTRIBUTES) return p;
    return L"D:/IndexIT_Final/data/small_set";
}

void test_binary_cache_roundtrip_and_corruption() {
    std::wstring dataset = resolve_dataset_path();
    std::wstring testCacheDir = L"./test_cache_temp";

    BinaryIndexCache::ensureDirectoryExists(testCacheDir);

    CppInvertedIndex index1;
    int ret = index1.indexDirectory(dataset);
    assert(ret == 0 && index1.getTotalDocs() == 5);

    bool saved = BinaryIndexCache::save(testCacheDir, index1);
    assert(saved);
    assert(BinaryIndexCache::getCacheFileSize(testCacheDir) > 0);

    CppInvertedIndex index2;
    bool loaded = BinaryIndexCache::load(testCacheDir, index2);
    assert(loaded);
    assert(index2.getTotalDocs() == 5);
    assert(std::fabs(index2.getAvgDocLength() - index1.getAvgDocLength()) < 0.001);

    Bm25Ranker ranker;
    auto res1 = index1.search("world", ranker);
    auto res2 = index2.search("world", ranker);
    assert(res1.size() == res2.size());
    assert(res1[0].docId == res2[0].docId);

    // Corrupt one byte of payload
    std::wstring cacheFilePath = testCacheDir + L"\\index.cache";
    HANDLE hFile = CreateFileW(cacheFilePath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    assert(hFile != INVALID_HANDLE_VALUE);
    SetFilePointer(hFile, 40, NULL, FILE_BEGIN);
    char corruptByte = 0x7F;
    DWORD written;
    WriteFile(hFile, &corruptByte, 1, &written, NULL);
    CloseHandle(hFile);

    CppInvertedIndex indexCorrupt;
    bool corruptLoaded = BinaryIndexCache::load(testCacheDir, indexCorrupt);
    assert(!corruptLoaded);

    bool cleaned = CacheManifest::clean(testCacheDir, true);
    assert(cleaned);

    RemoveDirectoryW(testCacheDir.c_str());
    std::cout << "BinaryIndexCache round-trip, corruption rejection, and manifest clean tests passed.\n";
}

void test_disk_space_guard() {
    std::wstring testCacheDir = L"./test_cache_guard";
    BinaryIndexCache::ensureDirectoryExists(testCacheDir);

    uint64_t impossibleThreshold = 10000ULL * 1024ULL * 1024ULL * 1024ULL * 1024ULL;
    bool pass = BinaryIndexCache::checkDiskSpace(testCacheDir, impossibleThreshold);
    assert(!pass);

    RemoveDirectoryW(testCacheDir.c_str());
    std::cout << "Disk-space guard tests passed.\n";
}

void test_sqlite_layer() {
    std::string testDb = "./test_sqlite.db";
    DeleteFileA(testDb.c_str());

    {
        // 1. Test RAII connection & schema initialization
        DatabaseManager db(testDb);

        // 2. Test saving and loading documents
        DocumentStore store1;
        store1.addDocument(L"C:\\files\\doc1.txt", 1024, 12345678, FORMAT_TXT);
        store1.setLength(1, 45);
        store1.addDocument(L"C:\\files\\doc2.md", 2048, 87654321, FORMAT_MD);
        store1.setLength(2, 90);

        db.saveDocuments(store1);

        DocumentStore store2;
        db.loadDocuments(store2);
        assert(store2.size() == 2);
        assert(store2.getPath(1) == L"C:\\files\\doc1.txt");
        assert(store2.getLength(1) == 45);
        assert(store2.getPath(2) == L"C:\\files\\doc2.md");
        assert(store2.getLength(2) == 90);

        // 3. Test manifest recording
        db.recordManifest("index.cache", 4096, "2026-10-29 23:00:00");

        // 4. Test search history
        db.recordSearchHistory("query one", 5);
        db.recordSearchHistory("query two", 12);
        db.recordSearchHistory("query three", 0);

        auto history = db.getSearchHistory(10);
        assert(history.size() == 3);
        assert(history[0].query == "query three" && history[0].resultCount == 0);
        assert(history[1].query == "query two" && history[1].resultCount == 12);
        assert(history[2].query == "query one" && history[2].resultCount == 5);

        // 5. Test transaction rollback
        {
            SqliteConnection conn(testDb);
            SqliteTransaction txn(conn.get());
            conn.execute("INSERT INTO search_history (query, ts, result_count) VALUES ('temp', 'now', 1);");
            txn.rollback();
        }
        auto historyAfterRollback = db.getSearchHistory(10);
        assert(historyAfterRollback.size() == 3); // 'temp' was rolled back
    }

    // 6. Test exception on invalid SQL
    bool threw = false;
    try {
        SqliteConnection conn(testDb);
        conn.execute("INVALID SQL SYNTAX HERE;");
    } catch (const SqliteException& e) {
        threw = true;
    }
    assert(threw);

    DeleteFileA(testDb.c_str());
    std::cout << "SQLite layer (RAII, transactions, schema, history, prepared statements) tests passed.\n";
}

int main() {
    test_extractors();
    test_fuzzy_matching();
    test_tokenizer();
    test_hashtable();
    test_trie_prefix_and_autocomplete();
    test_topk();
    test_cache_manager_byte_cap();
    test_scoped_timer();
    test_binary_cache_roundtrip_and_corruption();
    test_disk_space_guard();
    test_sqlite_layer();
    std::cout << "All tests passed successfully.\n";
    return 0;
}
