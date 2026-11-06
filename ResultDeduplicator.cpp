#include "ResultDeduplicator.hpp"
#include <set>
#include <algorithm>
#include <cctype>

namespace paras_advanced {

static std::string comparisonKey(const std::string& path) {
    std::string key = path;
    std::replace(key.begin(), key.end(), '\\', '/');

    for (std::string::size_type i = 0; i < key.size(); ++i) {
        unsigned char ch = static_cast<unsigned char>(key[i]);
        if (ch >= 'A' && ch <= 'Z') key[i] = static_cast<char>(ch - 'A' + 'a');
    }
    return key;
}

std::vector<RankedPath> ResultDeduplicator::removeDuplicatePaths(
    const std::vector<RankedPath>& results) const {
    std::set<std::string> seen;
    std::vector<RankedPath> unique;
    unique.reserve(results.size());

    for (std::vector<RankedPath>::const_iterator it = results.begin();
         it != results.end(); ++it) {
        const std::string key = comparisonKey(it->path);
        if (seen.insert(key).second) unique.push_back(*it);
    }
    return unique;
}

} // namespace paras_advanced
