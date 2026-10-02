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
    winfs_walk_directory(path.c_str(), cpp_index_callback, nullptr);
    std::wcout << L"Indexed " << idx.getTotalDocs() << L" documents.\n";
    extractor_cleanup();
}

int main(int argc_dummy, char** argv_dummy) {
    setvbuf(stdout, NULL, _IONBF, 0);
    int argc;
    wchar_t **argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv) return 1;

    if (argc < 2) {
        std::cout << "Usage: indexit <command> [args]\n";
        std::cout << "Commands:\n";
        std::cout << "  index <dir>\n";
        std::cout << "  search <dir> <term>\n";
        std::cout << "  ui <dir>\n";
        LocalFree(argv);
        return 1;
    }
    
    std::wstring cmd = argv[1];
    if (cmd == L"ui" && argc == 3) {
        ui::App app;
        app.doIndex(argv[2]);
        app.run();
    } else if (cmd == L"search" && argc == 4) {
        CppInvertedIndex idx;
        do_index_cpp(argv[2], idx);
        
        std::wstring wterm = argv[3];
        std::string term(wterm.begin(), wterm.end());
        
        Bm25Ranker ranker;
        auto results = idx.search(term, ranker);
        
        std::cout << "Found " << results.size() << " results for '" << term << "':\n";
        for (const auto& res : results) {
            std::cout << "  Doc ID: " << res.docId << " (Score: " << res.score << ")\n";
        }
    } else if (cmd == L"index" && argc == 3) {
        CppInvertedIndex idx;
        do_index_cpp(argv[2], idx);
    } else {
        std::cout << "Unknown command.\n";
    }
    
    LocalFree(argv);
    return 0;
}
