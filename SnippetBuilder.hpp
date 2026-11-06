#ifndef PARAS_PHASE3_SNIPPET_BUILDER_HPP
#define PARAS_PHASE3_SNIPPET_BUILDER_HPP

#include <string>
#include <vector>

namespace paras_phase3 {

struct Snippet {
    std::string text;
    std::vector<std::string> matchedTerms;
    std::size_t startOffset;
};

class SnippetBuilder {
public:
    Snippet build(const std::string& content,
                  const std::vector<std::string>& terms,
                  std::size_t maximumLength = 180) const;

private:
    static std::size_t findFirstMatch(
        const std::string& content,
        const std::vector<std::string>& terms);
};

} // namespace paras_phase3

#endif
