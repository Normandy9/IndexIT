#ifndef PARAS_ADVANCED_RESULT_DEDUPLICATOR_HPP
#define PARAS_ADVANCED_RESULT_DEDUPLICATOR_HPP

#include <string>
#include <vector>

namespace paras_advanced {

struct RankedPath {
    std::string path;
    double score;
};

class ResultDeduplicator {
public:
    std::vector<RankedPath> removeDuplicatePaths(
        const std::vector<RankedPath>& results) const;
};

} // namespace paras_advanced

#endif
