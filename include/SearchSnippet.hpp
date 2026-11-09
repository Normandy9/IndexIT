#pragma once

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>

inline std::string makeSearchSnippet(const std::string& text,
                                     const std::string& query,
                                     std::size_t context) {
    if (text.empty()) return {};

    std::string lowerText = text;
    std::transform(lowerText.begin(), lowerText.end(), lowerText.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    std::size_t match = std::string::npos;
    std::size_t matchLength = 0;
    std::istringstream words(query);
    std::string word;
    while (words >> word) {
        if (!word.empty() && word.back() == '*') word.pop_back();
        word.erase(std::remove_if(word.begin(), word.end(), [](unsigned char c) {
            return !std::isalnum(c);
        }), word.end());
        std::transform(word.begin(), word.end(), word.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        if (word.empty()) continue;

        const std::size_t found = lowerText.find(word);
        if (found != std::string::npos && found < match) {
            match = found;
            matchLength = word.size();
        }
    }

    if (match == std::string::npos) match = 0;
    const std::size_t start = match > context ? match - context : 0;
    const std::size_t end = std::min(text.size(), match + matchLength + context);
    std::string snippet = text.substr(start, end - start);
    if (start > 0) snippet.insert(0, "...");
    if (end < text.size()) snippet += "...";
    return snippet;
}
