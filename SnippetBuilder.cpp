#include "SnippetBuilder.hpp"
#include "StringUtilities.hpp"

#include <algorithm>

namespace paras_phase3 {

std::size_t SnippetBuilder::findFirstMatch(
    const std::string& content,
    const std::vector<std::string>& terms) {
    const std::string lowerContent = asciiLower(content);
    std::size_t earliest = std::string::npos;

    for (std::vector<std::string>::const_iterator it = terms.begin();
         it != terms.end(); ++it) {
        if (it->empty()) continue;
        const std::size_t found = lowerContent.find(asciiLower(*it));
        if (found != std::string::npos && found < earliest) earliest = found;
    }
    return earliest;
}

Snippet SnippetBuilder::build(const std::string& content,
                              const std::vector<std::string>& terms,
                              std::size_t maximumLength) const {
    Snippet result;
    result.startOffset = 0;

    if (maximumLength == 0 || content.empty()) {
        result.text = "";
        return result;
    }

    const std::size_t match = findFirstMatch(content, terms);
    std::size_t start = 0;

    if (match != std::string::npos && match > maximumLength / 3) {
        start = match - maximumLength / 3;
    }

    if (start > 0) {
        while (start < content.size() &&
               content[start] != ' ' && content[start] != '\n' &&
               content[start] != '\t') {
            ++start;
        }
        if (start < content.size()) ++start;
    }

    std::size_t end = start + maximumLength;
    if (end > content.size()) end = content.size();

    if (end < content.size()) {
        while (end > start && content[end] != ' ' &&
               content[end] != '\n' && content[end] != '\t') {
            --end;
        }
    }

    if (end <= start) {
        end = std::min(content.size(), start + maximumLength);
    }

    result.startOffset = start;
    result.text = content.substr(start, end - start);

    if (start > 0) result.text.insert(0, "... ");
    if (end < content.size()) result.text += " ...";

    const std::string lowerSnippet = asciiLower(result.text);
    for (std::vector<std::string>::const_iterator it = terms.begin();
         it != terms.end(); ++it) {
        if (it->empty()) continue;
        if (lowerSnippet.find(asciiLower(*it)) != std::string::npos) {
            result.matchedTerms.push_back(*it);
        }
    }

    return result;
}

} // namespace paras_phase3
