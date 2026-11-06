#ifndef PARAS_ADVANCED_SEARCH_ANALYTICS_HPP
#define PARAS_ADVANCED_SEARCH_ANALYTICS_HPP

#include <string>
#include <vector>
#include <cstddef>

namespace paras_advanced {

struct QueryEvent {
    std::string query;
    std::size_t resultCount;
    long long durationMilliseconds;
};

struct SearchSummary {
    std::size_t totalQueries;
    std::size_t successfulQueries;
    std::size_t emptyQueries;
    long long totalDurationMilliseconds;
    double averageDurationMilliseconds;
    double successRate;
};

class SearchAnalytics {
public:
    SearchSummary summarize(const std::vector<QueryEvent>& events) const;
    std::vector<std::string> mostFrequentQueries(
        const std::vector<QueryEvent>& events,
        std::size_t maximumCount) const;
};

} // namespace paras_advanced

#endif
