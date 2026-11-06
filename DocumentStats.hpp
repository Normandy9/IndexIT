#ifndef PARAS_ADVANCED_DOCUMENT_STATS_HPP
#define PARAS_ADVANCED_DOCUMENT_STATS_HPP

#include <string>
#include <cstddef>

namespace paras_advanced {

struct DocumentStatistics {
    std::size_t characters;
    std::size_t words;
    std::size_t lines;
    std::size_t nonEmptyLines;
    double averageWordLength;
};

class DocumentStats {
public:
    DocumentStatistics analyze(const std::string& text) const;
};

} // namespace paras_advanced

#endif
