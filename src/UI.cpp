#include "UI.hpp"
#include <iostream>
#include <iomanip>

namespace indexit {
namespace ui {

void StatusBar::draw(HANDLE hConsole, int width, int height) {
    COORD pos = {0, static_cast<SHORT>(height - 1)};
    SetConsoleCursorPosition(hConsole, pos);
    SetConsoleTextAttribute(hConsole, ThemeManager::getStatusColor());
    
    std::string line = text_;
    if (line.size() < static_cast<size_t>(width)) line.append(width - line.size(), ' ');
    else if (line.size() > static_cast<size_t>(width)) line = line.substr(0, width);
    
    DWORD written;
    WriteConsoleA(hConsole, line.c_str(), static_cast<DWORD>(line.size()), &written, nullptr);
    SetConsoleTextAttribute(hConsole, ThemeManager::getDefaultColor());
}

void ResultView::draw(HANDLE hConsole, int width, int startY, int maxLines) {
    SetConsoleTextAttribute(hConsole, ThemeManager::getDefaultColor());
    for (int i = 0; i < maxLines; i++) {
        COORD pos = {0, static_cast<SHORT>(startY + i)};
        SetConsoleCursorPosition(hConsole, pos);
        std::string clearLine(width > 0 ? width : 1, ' ');
        DWORD written;
        WriteConsoleA(hConsole, clearLine.c_str(), static_cast<DWORD>(clearLine.size()), &written, nullptr);
        SetConsoleCursorPosition(hConsole, pos);
        
        if (static_cast<size_t>(i) < results_.size()) {
            const auto& res = results_[i];
            std::wstring wpath = index_->getPath(res.docId);
            std::string path(wpath.begin(), wpath.end());
            
            SetConsoleTextAttribute(hConsole, ThemeManager::getHighlightColor());
            std::string scoreStr = std::to_string(res.score);
            WriteConsoleA(hConsole, scoreStr.c_str(), static_cast<DWORD>(scoreStr.size()), &written, nullptr);
            
            SetConsoleTextAttribute(hConsole, ThemeManager::getDefaultColor());
            std::string text = " - " + path;
            if (width > 10 && text.size() > static_cast<size_t>(width - 10)) {
                text = text.substr(0, width - 10);
            }
            WriteConsoleA(hConsole, text.c_str(), static_cast<DWORD>(text.size()), &written, nullptr);
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
    COORD pos = {0, 0};
    SetConsoleCursorPosition(hConsole, pos);
    SetConsoleTextAttribute(hConsole, ThemeManager::getDefaultColor());
    
    std::string prompt = "Search: " + query_;
    std::string clearLine = prompt;
    if (clearLine.size() < static_cast<size_t>(width)) clearLine.append(width - clearLine.size(), ' ');
    
    DWORD written;
    WriteConsoleA(hConsole, clearLine.c_str(), static_cast<DWORD>(clearLine.size()), &written, nullptr);
    
    pos.X = static_cast<SHORT>(prompt.size());
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

void App::doIndex(const std::wstring& path) {
    status_.setText("Indexing...");
    render();
    index_.indexDirectory(path);
    status_.setText("Ready. Indexed " + std::to_string(index_.getTotalDocs()) + " documents.");
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
    GetConsoleScreenBufferInfo(hConsole_, &csbi);
    int width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    if (width <= 0) width = 80;
    if (height <= 0) height = 25;
    
    input_.draw(hConsole_, width);

    // Live suggestions bar on line 1
    COORD sugPos = {0, 1};
    SetConsoleCursorPosition(hConsole_, sugPos);
    std::string q = input_.getQuery();
    std::string lastWord;
    if (!q.empty()) {
        size_t lastSpace = q.find_last_of(" \t");
        if (lastSpace == std::string::npos) lastWord = q;
        else lastWord = q.substr(lastSpace + 1);
    }

    std::string sugLine = "Suggestions: ";
    if (!lastWord.empty()) {
        auto suggestions = index_.suggest(lastWord, 4);
        if (!suggestions.empty()) {
            for (const auto& s : suggestions) {
                sugLine += s.first + " (" + std::to_string(s.second) + ")  ";
            }
        } else {
            sugLine += "(none)";
        }
    } else {
        sugLine += "(type to see live suggestions)";
    }

    if (sugLine.size() < static_cast<size_t>(width)) sugLine.append(width - sugLine.size(), ' ');
    else if (sugLine.size() > static_cast<size_t>(width)) sugLine = sugLine.substr(0, width);

    SetConsoleTextAttribute(hConsole_, ThemeManager::getHighlightColor());
    DWORD written;
    WriteConsoleA(hConsole_, sugLine.c_str(), static_cast<DWORD>(sugLine.size()), &written, nullptr);
    SetConsoleTextAttribute(hConsole_, ThemeManager::getDefaultColor());

    results_.draw(hConsole_, width, 2, height - 3);
    status_.draw(hConsole_, width, height);
    
    // reset cursor to input
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
