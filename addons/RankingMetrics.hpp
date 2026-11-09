#ifndef PARAS_ADVANCED_RANKING_METRICS_HPP
#define PARAS_ADVANCED_RANKING_METRICS_HPP

#include <vector>
#include <string>

namespace paras_advanced {

struct RankingScore {
    double precision;
    double recall;
    double f1;
    std::size_t relevantRetrieved;
    std::size_t retrieved;
    std::size_t relevantTotal;
};

class RankingMetrics {
public:
    RankingScore evaluate(
        const std::vector<std::string>& retrievedIds,
        const std::vector<std::string>& relevantIds) const;
};

} // namespace paras_advanced

#endif
