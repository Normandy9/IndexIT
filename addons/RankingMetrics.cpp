#include "RankingMetrics.hpp"
#include <set>

namespace paras_advanced {

RankingScore RankingMetrics::evaluate(
    const std::vector<std::string>& retrievedIds,
    const std::vector<std::string>& relevantIds) const {
    std::set<std::string> relevant(relevantIds.begin(), relevantIds.end());
    std::set<std::string> retrievedUnique;
    std::size_t matches = 0;

    for (std::vector<std::string>::const_iterator it = retrievedIds.begin();
         it != retrievedIds.end(); ++it) {
        if (retrievedUnique.insert(*it).second && relevant.count(*it) != 0) {
            ++matches;
        }
    }

    RankingScore score;
    score.relevantRetrieved = matches;
    score.retrieved = retrievedUnique.size();
    score.relevantTotal = relevant.size();
    score.precision = score.retrieved == 0 ? 0.0 :
        static_cast<double>(matches) / static_cast<double>(score.retrieved);
    score.recall = score.relevantTotal == 0 ? 0.0 :
        static_cast<double>(matches) / static_cast<double>(score.relevantTotal);
    score.f1 = (score.precision + score.recall) == 0.0 ? 0.0 :
        2.0 * score.precision * score.recall /
        (score.precision + score.recall);
    return score;
}

} // namespace paras_advanced
