#ifndef PARAS_SEARCH_SUGGESTION_ENGINE_HPP
#define PARAS_SEARCH_SUGGESTION_ENGINE_HPP
#include <cstddef>
#include <string>
#include <vector>
namespace paras {
struct Suggestion {
    std::string term;
    std::size_t frequency;
};
class SearchSuggestionEngine {
private:
    struct Entry {
        std::string term;
        std::size_t frequency;
    };
    std::vector<Entry> entries;
    static std::string normalize(const std::string& value);
public:
    void add(const std::string& term, std::size_t frequency = 1);
    bool remove(const std::string& term);
    std::vector<Suggestion> suggest(const std::string& prefix,
                                    std::size_t limit = 5) const;
    std::size_t size() const;
    bool empty() const;
    void clear();
};
}
#endif
