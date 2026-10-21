#include <iostream>
#include <string>
#include <windows.h>
#include "CppInvertedIndex.hpp"
#include "UI.hpp"
#include "RAII.hpp"
#include <shellapi.h>

extern "C" {
#include "winfs.h"
#include "extractor.h"
#include "token.h"
}

using namespace indexit;

static CppInvertedIndex* g_index_ptr = nullptr;

static int cpp_index_callback(const Document* doc, void* user_data) {
    if (doc->format == FORMAT_UNKNOWN) return 0;
    
    char* text = extract_text(doc);
    if (text) {
        uint64_t docId = g_index_ptr->addDocument(doc->path);
        
        Tokenizer* tok = tokenizer_create(text);
        uint32_t len = 0;
        if (tok) {
            Token token;
            while (tokenizer_next(tok, &token)) {
                g_index_ptr->addTerm(token.text, docId, 1);
                free(token.text);
                len++;
            }
            tokenizer_destroy(tok);
        }
        g_index_ptr->setDocumentLength(docId, len);
        free(text);
    }
    return 0;
}

void do_index_cpp(const std::wstring& path, CppInvertedIndex& idx) {
    g_index_ptr = &idx;
    extractor_init();
    std::wcout << L"Indexing " << path << L"...\n";
    DWORD attr = GetFileAttributesW(path.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
        std::wcout << L"Error: Directory not found or is invalid: " << path << L"\n";
        return;
    }
    winfs_walk_directory(path.c_str(), cpp_index_callback, nullptr);
    std::wcout << L"Indexed " << idx.getTotalDocs() << L" documents.\n";
    extractor_cleanup();
}

void print_help() {
    std::cout << "IndexIt CLI\n";
    std::cout << "Usage: indexit <command> [args]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  index <dir>         - Index a directory into memory\n";
    std::cout << "  search <dir> <term> - Search a directory for a specific term/phrase\n";
    std::cout << "  ui <dir>            - Launch interactive terminal UI\n";
    std::cout << "  stats <dir>         - Display statistics for the indexed directory\n";
    std::cout << "  clean               - Clear caches (Stub for persistence)\n";
    std::cout << "  --help, -h          - Show this help message\n";
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
    } else if (cmd == L"ui" && argc == 3) {
        ui::App app;
        app.doIndex(argv[2]);
        app.run();
    } else if (cmd == L"search" && argc >= 4) {
        CppInvertedIndex idx;
        do_index_cpp(argv[2], idx);
        if (idx.getTotalDocs() == 0) {
            std::cout << "Index is empty. No results can be found.\n";
        } else {
            // Reconstruct full query (handling spaces if passed without quotes)
            std::wstring wterm = argv[3];
            for (int i = 4; i < argc; i++) {
                wterm += L" ";
                wterm += argv[i];
            }
            std::string term(wterm.begin(), wterm.end());
            
            Bm25Ranker ranker;
            auto results = idx.search(term, ranker);
            
            std::cout << "Found " << results.size() << " results for '" << term << "':\n";
            for (const auto& res : results) {
                std::wcout << L"  Doc ID: " << res.docId << L" (Score: " << res.score << L") - " << idx.getPath(res.docId) << L"\n";
            }
        }
    } else if (cmd == L"index" && argc == 3) {
        CppInvertedIndex idx;
        do_index_cpp(argv[2], idx);
    } else if (cmd == L"stats" && argc == 3) {
        CppInvertedIndex idx;
        do_index_cpp(argv[2], idx);
        std::cout << "--- Index Statistics ---\n";
        std::cout << "Total Documents: " << idx.getTotalDocs() << "\n";
        std::cout << "Average Doc Length: " << idx.getAvgDocLength() << "\n";
    } else if (cmd == L"clean") {
        std::cout << "Clean command: In-memory structures freed. (SQLite persistence clearing is stubbed).\n";
    } else {
        std::cout << "Error: Unknown or incomplete command.\n";
        print_help();
    }
    
    LocalFree(argv);
    return 0;
}
