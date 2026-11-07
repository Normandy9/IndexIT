#include "SearchSuggestionEngine.hpp"
#include <algorithm>
#include <cctype>
namespace paras {
std::string SearchSuggestionEngine::normalize(const std::string& value) {
    std::string result = value;
    for (std::size_t i = 0; i < result.size(); ++i) {
        result[i] = static_cast<char>(
            std::tolower(static_cast<unsigned char>(result[i])));
    }
    return result;
}
void SearchSuggestionEngine::add(const std::string& term, std::size_t frequency) {
    if (term.empty() || frequency == 0) return;
    std::string key = normalize(term);
    for (std::size_t i = 0; i < entries.size(); ++i) {
        if (normalize(entries[i].term) == key) {
            entries[i].frequency += frequency;
            return;
        }
    }
    entries.push_back({term, frequency});
}
bool SearchSuggestionEngine::remove(const std::string& term) {
    std::string key = normalize(term);
    for (std::vector<Entry>::iterator it = entries.begin(); it != entries.end(); ++it) {
        if (normalize(it->term) == key) {
            entries.erase(it);
            return true;
        }
    }
    return false;
}
std::vector<Suggestion> SearchSuggestionEngine::suggest(
        const std::string& prefix, std::size_t limit) const {
    std::vector<Suggestion> result;
    std::string key = normalize(prefix);
    for (std::size_t i = 0; i < entries.size(); ++i) {
        std::string candidate = normalize(entries[i].term);
        if (candidate.size() >= key.size() &&
            candidate.compare(0, key.size(), key) == 0) {
            result.push_back({entries[i].term, entries[i].frequency});
        }
    }
    std::sort(result.begin(), result.end(),
        [](const Suggestion& a, const Suggestion& b) {
            if (a.frequency != b.frequency) return a.frequency > b.frequency;
            return normalize(a.term) < normalize(b.term);
        });
    if (result.size() > limit) result.resize(limit);
    return result;
}
std::size_t SearchSuggestionEngine::size() const { return entries.size(); }
bool SearchSuggestionEngine::empty() const { return entries.empty(); }
void SearchSuggestionEngine::clear() { entries.clear(); }
}
