#include "DocumentStats.hpp"
#include <cctype>

namespace paras_advanced {

DocumentStatistics DocumentStats::analyze(const std::string& text) const {
    DocumentStatistics result;
    result.characters = text.size();
    result.words = 0;
    result.lines = text.empty() ? 0 : 1;
    result.nonEmptyLines = 0;
    result.averageWordLength = 0.0;

    bool insideWord = false;
    bool lineHasContent = false;
    std::size_t currentWordLength = 0;
    std::size_t totalWordCharacters = 0;

    for (std::size_t i = 0; i < text.size(); ++i) {
        const unsigned char ch = static_cast<unsigned char>(text[i]);

        if (ch == '\n') {
            if (lineHasContent) ++result.nonEmptyLines;
            lineHasContent = false;
            ++result.lines;
        } else if (!std::isspace(ch)) {
            lineHasContent = true;
        }

        if (std::isspace(ch)) {
            if (insideWord) {
                ++result.words;
                totalWordCharacters += currentWordLength;
                currentWordLength = 0;
                insideWord = false;
            }
        } else {
            insideWord = true;
            ++currentWordLength;
        }
    }

    if (insideWord) {
        ++result.words;
        totalWordCharacters += currentWordLength;
    }
    if (lineHasContent) ++result.nonEmptyLines;

    if (result.words != 0) {
        result.averageWordLength =
            static_cast<double>(totalWordCharacters) /
            static_cast<double>(result.words);
    }

    return result;
}

} // namespace paras_advanced
