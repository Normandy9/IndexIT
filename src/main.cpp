#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cctype>
#include <windows.h>
#include <shellapi.h>
#include "CppInvertedIndex.hpp"
#include "UI.hpp"
#include "RAII.hpp"
#include "BinaryIndexCache.hpp"
#include "db.h"
#include "SearchFilters.hpp"
#include <cwchar>

using namespace indexit;

void print_help() {
    std::cout << "IndexIt CLI\n";
    std::cout << "Usage: indexit <command> [args]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  index <dir> [--cache-dir <path>]          - Index a directory (incremental caching)\n";
    std::cout << "  search <dir> <query> [--ext .pdf] [--path-contains text] [--min-size bytes] [--max-size bytes]\n";
    std::cout << "                                             - Search with optional metadata filters\n";
    std::cout << "  suggest <dir> <prefix> [--cache-dir <path>] - Autocomplete suggestions from Trie\n";
    std::cout << "  history [--cache-dir <path>]                - View SQLite search query history\n";
    std::cout << "  ui <dir>                                  - Launch interactive terminal UI\n";
    std::cout << "  stats <dir> [--cache-dir <path>]          - Show index, SQLite & disk cache statistics\n";
    std::cout << "  clean [--cache-dir <path>] [--yes]        - Delete index cache files after confirmation\n";
    std::cout << "  --help, -h                                - Show this help message\n";
}

int main(int argc_dummy, char** argv_dummy) {
    setvbuf(stdout, NULL, _IONBF, 0);
    int argc;
    wchar_t **argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv) return 1;

    if (argc < 2) {
        print_help();
        LocalFree(argv);
        return 1;
    }
    
    std::wstring cmd = argv[1];
    if (cmd == L"--help" || cmd == L"-h") {
        print_help();
        LocalFree(argv);
        return 0;
    }

    // Parse flags: --cache-dir <path>, --yes
    std::wstring cacheDir = BinaryIndexCache::getDefaultCacheDir();
    bool yesFlag = false;
    SearchFilters searchFilters;
    std::vector<std::wstring> positionalArgs;

    for (int i = 2; i < argc; i++) {
        std::wstring arg = argv[i];
        if (arg == L"--cache-dir" && i + 1 < argc) {
            cacheDir = argv[++i];
        } else if (arg == L"--yes" || arg == L"-y") {
            yesFlag = true;
        } else if (arg == L"--ext" && i + 1 < argc) {
            searchFilters.extension = argv[++i];
        } else if (arg == L"--path-contains" && i + 1 < argc) {
            searchFilters.pathContains = argv[++i];
        } else if (arg == L"--min-size" && i + 1 < argc) {
            wchar_t* end = nullptr; unsigned long long v = wcstoull(argv[++i], &end, 10);
            if (!end || *end != 0) { std::wcerr << L"Invalid --min-size value\n"; LocalFree(argv); return 2; }
            searchFilters.minSize = static_cast<uint64_t>(v);
        } else if (arg == L"--max-size" && i + 1 < argc) {
            wchar_t* end = nullptr; unsigned long long v = wcstoull(argv[++i], &end, 10);
            if (!end || *end != 0) { std::wcerr << L"Invalid --max-size value\n"; LocalFree(argv); return 2; }
            searchFilters.maxSize = static_cast<uint64_t>(v);
        } else {
            positionalArgs.push_back(arg);
        }
    }

    std::string cacheDirA(cacheDir.begin(), cacheDir.end());
    std::string dbPath = cacheDirA + "\\indexit.db";

    if (cmd == L"clean") {
        CacheManifest::clean(cacheDir, yesFlag);
    } else if (cmd == L"history") {
        try {
            DatabaseManager db(dbPath);
            auto history = db.getSearchHistory(50);
            if (history.empty()) {
                std::cout << "No search history recorded yet.\n";
            } else {
                std::cout << "--- Search History (most recent first) ---\n";
                for (const auto& h : history) {
                    std::cout << "[" << h.id << "] Query: '" << h.query << "' | Results: "
                              << h.resultCount << " | Time: " << h.timestamp << "\n";
                }
            }
        } catch (const std::exception& e) {
            std::cout << "Error reading search history: " << e.what() << "\n";
        }
    } else if (cmd == L"ui") {
        if (positionalArgs.empty()) {
            std::cout << "Error: Directory path required for UI mode.\n";
        } else {
            ui::App app;
            app.doIndex(positionalArgs[0]);
            app.run();
        }
    } else if (cmd == L"index") {
        if (positionalArgs.empty()) {
            std::cout << "Error: Directory path required for indexing.\n";
        } else {
            CppInvertedIndex idx;
            std::wcout << L"Indexing " << positionalArgs[0] << L" (Cache: " << cacheDir << L")...\n";
            int ret = BinaryIndexCache::updateIndexIncremental(positionalArgs[0], cacheDir, idx);
            if (ret != 0) {
                std::wcout << L"Error: Failed to index directory: " << positionalArgs[0] << L"\n";
            } else {
                try {
                    DatabaseManager db(dbPath);
                    db.saveDocuments(idx.getDocumentStore());
                    CacheManifest::recordFile(cacheDir, L"indexit.db", BinaryIndexCache::getCacheFileSize(cacheDir));
                } catch (const std::exception& e) {
                    std::cout << "[WARN] SQLite metadata save warning: " << e.what() << "\n";
                }
                std::wcout << L"Indexed " << idx.getTotalDocs() << L" documents. Cache & SQLite DB updated.\n";
            }
        }
    } else if (cmd == L"search") {
        if (positionalArgs.size() < 2) {
            std::cout << "Error: Directory and search query required.\n";
        } else {
            CppInvertedIndex idx;
            if (!BinaryIndexCache::load(cacheDir, idx) || idx.getTotalDocs() == 0) {
                BinaryIndexCache::updateIndexIncremental(positionalArgs[0], cacheDir, idx);
            }

            if (idx.getTotalDocs() == 0) {
                std::cout << "Index is empty. No results can be found.\n";
            } else {
                std::wstring wquery = positionalArgs[1];
                for (size_t i = 2; i < positionalArgs.size(); i++) {
                    wquery += L" " + positionalArgs[i];
                }
                std::string query(wquery.begin(), wquery.end());

                Bm25Ranker ranker;
                auto results = idx.search(query, ranker);
                std::vector<ScoredDocument> filteredResults;
                for (const auto& res : results) {
                    std::wstring path = idx.getPath(res.docId);
                    WIN32_FILE_ATTRIBUTE_DATA fileData;
                    uint64_t size = 0;
                    if (GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &fileData)) {
                        size = (static_cast<uint64_t>(fileData.nFileSizeHigh) << 32) | fileData.nFileSizeLow;
                    }
                    if (searchFilters.matches(path, size)) filteredResults.push_back(res);
                }
                results.swap(filteredResults);
                if (searchFilters.minSize > searchFilters.maxSize) {
                    std::cout << "Invalid filters: --min-size cannot exceed --max-size.\n";
                    return 2;
                }
                std::cout << "Found " << results.size() << " results for '" << query << "' after metadata filters:\n";
                for (const auto& res : results) {
                    std::wcout << L"  Doc ID: " << res.docId << L" (Score: " << res.score << L") - "
                               << idx.getPath(res.docId) << L"\n";
                    std::string snippet = idx.getSnippet(res.docId, query, 65);
                    if (!snippet.empty()) std::cout << "    " << snippet << "\n";
                }
                if (results.empty()) {
                    std::istringstream words(query);
                    std::string word;
                    while (words >> word) {
                        for (size_t c=0;c<word.size();++c) word[c]=static_cast<char>(tolower(static_cast<unsigned char>(word[c])));
                        if (idx.docFrequency(word) == 0) {
                            auto alternatives=idx.suggestSimilar(word,3);
                            if (!alternatives.empty()) {
                                std::cout << "Did you mean " << word << "? ";
                                for(size_t j=0;j<alternatives.size();++j) std::cout << (j?", ":"") << alternatives[j].first;
                                std::cout << "\n";
                            }
                        }
                    }
                }

                // Record search into SQLite history
                try {
                    DatabaseManager db(dbPath);
                    db.recordSearchHistory(query, static_cast<int>(results.size()));
                } catch (...) {
                    // Non-fatal
                }
            }
        }
    } else if (cmd == L"suggest") {
        if (positionalArgs.size() < 2) {
            std::cout << "Error: Directory and prefix term required.\n";
        } else {
            CppInvertedIndex idx;
            if (!BinaryIndexCache::load(cacheDir, idx) || idx.getTotalDocs() == 0) {
                BinaryIndexCache::updateIndexIncremental(positionalArgs[0], cacheDir, idx);
            }

            std::wstring wprefix = positionalArgs[1];
            std::string prefix(wprefix.begin(), wprefix.end());
            auto suggestions = idx.suggest(prefix, 10);
            if (suggestions.empty()) {
                std::cout << "No suggestions found for '" << prefix << "'.\n";
            } else {
                std::cout << "Suggestions for '" << prefix << "':\n";
                for (const auto& s : suggestions) {
                    std::cout << "  " << s.first << " (doc frequency: " << s.second << ")\n";
                }
            }
        }
    } else if (cmd == L"stats") {
        if (positionalArgs.empty()) {
            std::cout << "Error: Directory path required for stats.\n";
        } else {
            CppInvertedIndex idx;
            if (!BinaryIndexCache::load(cacheDir, idx) || idx.getTotalDocs() == 0) {
                BinaryIndexCache::updateIndexIncremental(positionalArgs[0], cacheDir, idx);
            }

            uint64_t cacheFileSize = BinaryIndexCache::getCacheFileSize(cacheDir);
            uint64_t freeSpace = BinaryIndexCache::getAvailableDiskSpace(cacheDir);

            std::cout << "--- Index, Cache & SQLite Statistics ---\n";
            std::wcout << L"Indexed Directory: " << positionalArgs[0] << L"\n";
            std::wcout << L"Cache Directory:   " << cacheDir << L"\n";
            std::cout << "Total Documents:     " << idx.getTotalDocs() << "\n";
            std::cout << "Total Unique Terms:  " << (idx.getExactIndex() ? idx.getExactIndex()->size() : 0) << "\n";
            std::cout << "Average Doc Length:  " << idx.getAvgDocLength() << "\n";
            if (cacheFileSize > 0) {
                std::cout << "Binary Cache Size:   " << cacheFileSize << " bytes ("
                          << (cacheFileSize / 1024.0) << " KB)\n";
            } else {
                std::cout << "Binary Cache Size:   Not saved to disk yet\n";
            }
            std::cout << "Free Disk Space:     " << (freeSpace / (1024 * 1024)) << " MB available\n";

            try {
                DatabaseManager db(dbPath);
                auto history = db.getSearchHistory(1);
                std::cout << "Search History:      Available (run 'indexit history' to view)\n";
            } catch (...) {
                std::cout << "Search History:      Database not initialized\n";
            }
        }
    } else {
        std::cout << "Error: Unknown command: " << std::string(cmd.begin(), cmd.end()) << "\n";
        print_help();
    }

    LocalFree(argv);
    return 0;
}
