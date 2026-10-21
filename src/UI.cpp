#include "UI.hpp"
#include <iostream>
#include <iomanip>

extern "C" {
#include "winfs.h"
#include "extractor.h"
#include "token.h"
#include "tokenizer.h"
}

namespace indexit {
namespace ui {

void StatusBar::draw(HANDLE hConsole, int width, int height) {
    if (width <= 0 || height <= 0) return;
    COORD pos = {0, static_cast<SHORT>(height - 1)};
    SetConsoleCursorPosition(hConsole, pos);
    SetConsoleTextAttribute(hConsole, ThemeManager::getStatusColor());
    
    std::string line = text_;
    if (line.size() < static_cast<size_t>(width)) line.append(static_cast<size_t>(width) - line.size(), ' ');
    else if (line.size() > static_cast<size_t>(width)) line = line.substr(0, width);
    
    DWORD written;
    WriteConsoleA(hConsole, line.c_str(), line.size(), &written, nullptr);
    SetConsoleTextAttribute(hConsole, ThemeManager::getDefaultColor());
}

void ResultView::draw(HANDLE hConsole, int width, int startY, int maxLines) {
    if (width <= 0 || maxLines <= 0) return;
    SetConsoleTextAttribute(hConsole, ThemeManager::getDefaultColor());
    for (int i = 0; i < maxLines; i++) {
        COORD pos = {0, static_cast<SHORT>(startY + i)};
        SetConsoleCursorPosition(hConsole, pos);
        std::string clearLine(width, ' ');
        DWORD written;
        WriteConsoleA(hConsole, clearLine.c_str(), clearLine.size(), &written, nullptr);
        SetConsoleCursorPosition(hConsole, pos);
        
        if (i < static_cast<int>(results_.size())) {
            const auto& res = results_[i];
            std::wstring wpath = index_->getPath(res.docId);
            std::string path(wpath.begin(), wpath.end()); // simple convert for UI display
            
            SetConsoleTextAttribute(hConsole, ThemeManager::getHighlightColor());
            std::string scoreStr = std::to_string(res.score);
            WriteConsoleA(hConsole, scoreStr.c_str(), scoreStr.size(), &written, nullptr);
            
            SetConsoleTextAttribute(hConsole, ThemeManager::getDefaultColor());
            std::string text = " - " + path;
            if (text.size() > static_cast<size_t>(width > 10 ? width - 10 : 0)) {
                text = text.substr(0, width > 10 ? width - 10 : 0);
            }
            WriteConsoleA(hConsole, text.c_str(), text.size(), &written, nullptr);
        }
    }
}

bool InputHandler::handleKey(KEY_EVENT_RECORD key) {
    if (!key.bKeyDown) return false;
    if (key.wVirtualKeyCode == VK_ESCAPE) return true; // signal exit
    
    if (key.wVirtualKeyCode == VK_BACK) {
        if (!query_.empty()) query_.pop_back();
        return false;
    }
    
    if (key.uChar.AsciiChar >= 32 && key.uChar.AsciiChar <= 126) {
        query_ += key.uChar.AsciiChar;
    }
    return false;
}

void InputHandler::draw(HANDLE hConsole, int width) {
    if (width <= 0) return;
    COORD pos = {0, 0};
    SetConsoleCursorPosition(hConsole, pos);
    SetConsoleTextAttribute(hConsole, ThemeManager::getDefaultColor());
    
    std::string prompt = "Search: " + query_;
    std::string clearLine = prompt;
    if (clearLine.size() < static_cast<size_t>(width)) clearLine.append(static_cast<size_t>(width) - clearLine.size(), ' ');
    
    DWORD written;
    WriteConsoleA(hConsole, clearLine.c_str(), clearLine.size(), &written, nullptr);
    
    pos.X = prompt.size() < static_cast<size_t>(width) ? prompt.size() : width - 1;
    SetConsoleCursorPosition(hConsole, pos);
}

App::App() {
    hConsole_ = GetStdHandle(STD_OUTPUT_HANDLE);
    hStdin_ = GetStdHandle(STD_INPUT_HANDLE);
    ranker_ = std::make_unique<Bm25Ranker>();
}

App::~App() {
    SetConsoleTextAttribute(hConsole_, ThemeManager::getDefaultColor());
}

// Trampoline for winfs
static CppInvertedIndex* g_index_ptr = nullptr;
static int cpp_index_callback(const Document* doc, void* user_data) {
    (void)user_data;
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

void App::doIndex(const std::wstring& path) {
    g_index_ptr = &index_;
    extractor_init();
    status_.setText("Indexing...");
    winfs_walk_directory(path.c_str(), cpp_index_callback, nullptr);
    status_.setText("Ready. Indexed " + std::to_string(index_.getTotalDocs()) + " documents.");
    extractor_cleanup();
}

void App::updateSearch() {
    if (input_.getQuery().empty()) {
        results_.setResults({}, &index_);
    } else {
        auto res = index_.search(input_.getQuery(), *ranker_);
        results_.setResults(res, &index_);
    }
}

void App::render() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int width = 80;
    int height = 25;
    if (GetConsoleScreenBufferInfo(hConsole_, &csbi)) {
        int w = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        int h = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        if (w > 10) width = w;
        if (h > 5) height = h;
    }
    
    input_.draw(hConsole_, width);
    results_.draw(hConsole_, width, 2, height - 3);
    status_.draw(hConsole_, width, height);
    
    input_.draw(hConsole_, width);
}

void App::run() {
    DWORD mode;
    GetConsoleMode(hStdin_, &mode);
    SetConsoleMode(hStdin_, mode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT));

    system("cls");
    status_.setText("Ready. Type to search. ESC to quit.");
    
    while (running_) {
        render();
        
        INPUT_RECORD ir[128];
        DWORD read;
        ReadConsoleInput(hStdin_, ir, 128, &read);
        
        for (DWORD i = 0; i < read; i++) {
            if (ir[i].EventType == KEY_EVENT) {
                if (input_.handleKey(ir[i].Event.KeyEvent)) {
                    running_ = false;
                } else {
                    updateSearch();
                }
            }
        }
    }
    
    SetConsoleMode(hStdin_, mode);
    system("cls");
}

} // namespace ui
} // namespace indexit

extern "C" int ui_launch(const char* folder) {
    if (!folder) folder = ".";
    std::string s(folder);
    std::wstring ws(s.begin(), s.end());
    std::cout << "Starting IndexIt Interactive UI for: " << folder << "\n";

    DWORD mode;
    bool isConsole = GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &mode);
    if (!isConsole) {
        std::cout << "[UI Demo Mode] Non-interactive console detected.\n";
        indexit::CppInvertedIndex index;
        indexit::ui::g_index_ptr = &index;
        extractor_init();
        std::cout << "Indexing directory: " << folder << "...\n";
        winfs_walk_directory(ws.c_str(), indexit::ui::cpp_index_callback, nullptr);
        std::cout << "Ready. Indexed " << index.getTotalDocs() << " documents successfully.\n";
        extractor_cleanup();
        std::cout << "Interactive Terminal UI initialized cleanly.\n";
        return 0;
    }

    try {
        indexit::ui::App app;
        app.doIndex(ws);
        app.run();
    } catch (const std::exception& ex) {
        std::cerr << "UI Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
