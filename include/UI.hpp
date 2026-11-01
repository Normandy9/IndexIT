#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <functional>
#include "CppInvertedIndex.hpp"

namespace indexit {
namespace ui {

class ThemeManager {
public:
    static WORD getDefaultColor() { return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; }
    static WORD getHighlightColor() { return FOREGROUND_GREEN | FOREGROUND_INTENSITY; }
    static WORD getStatusColor() { return BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY; }
};

class StatusBar {
    std::string text_;
public:
    void setText(const std::string& t) { text_ = t; }
    void draw(HANDLE hConsole, int width, int height);
};

class ResultView {
    std::vector<ScoredDocument> results_;
    CppInvertedIndex* index_;
public:
    void setResults(const std::vector<ScoredDocument>& r, CppInvertedIndex* idx) { results_ = r; index_ = idx; }
    void draw(HANDLE hConsole, int width, int startY, int maxLines);
};

class InputHandler {
    std::string query_;
public:
    bool handleKey(KEY_EVENT_RECORD key);
    const std::string& getQuery() const { return query_; }
    void draw(HANDLE hConsole, int width);
};

class App {
    HANDLE hConsole_;
    HANDLE hStdin_;
    bool running_ = true;
    InputHandler input_;
    ResultView results_;
    StatusBar status_;
    CppInvertedIndex index_;
    std::unique_ptr<IRanker> ranker_;
    
    void render();
    void updateSearch();
public:
    App();
    ~App();
    void run();
    void doIndex(const std::wstring& path);
};

} // namespace ui
} // namespace indexit
