#include "SearchAnalytics.hpp"
#include <map>
#include <algorithm>

namespace paras_advanced {

SearchSummary SearchAnalytics::summarize(
    const std::vector<QueryEvent>& events) const {
    SearchSummary summary;
    summary.totalQueries = events.size();
    summary.successfulQueries = 0;
    summary.emptyQueries = 0;
    summary.totalDurationMilliseconds = 0;
    summary.averageDurationMilliseconds = 0.0;
    summary.successRate = 0.0;

    for (std::vector<QueryEvent>::const_iterator it = events.begin();
         it != events.end(); ++it) {
        if (it->resultCount > 0) ++summary.successfulQueries;
        else ++summary.emptyQueries;

        if (it->durationMilliseconds > 0) {
            summary.totalDurationMilliseconds += it->durationMilliseconds;
        }
    }

    if (!events.empty()) {
        summary.averageDurationMilliseconds =
            static_cast<double>(summary.totalDurationMilliseconds) /
            static_cast<double>(events.size());
        summary.successRate =
            100.0 * static_cast<double>(summary.successfulQueries) /
            static_cast<double>(events.size());
    }

    return summary;
}

std::vector<std::string> SearchAnalytics::mostFrequentQueries(
    const std::vector<QueryEvent>& events,
    std::size_t maximumCount) const {
    std::map<std::string, std::size_t> counts;

    for (std::vector<QueryEvent>::const_iterator it = events.begin();
         it != events.end(); ++it) {
        if (!it->query.empty()) ++counts[it->query];
    }

    std::vector<std::pair<std::string, std::size_t> > ordered(
        counts.begin(), counts.end());

    std::sort(ordered.begin(), ordered.end(),
        [](const std::pair<std::string, std::size_t>& a,
           const std::pair<std::string, std::size_t>& b) {
            if (a.second != b.second) return a.second > b.second;
            return a.first < b.first;
        });

    std::vector<std::string> result;
    const std::size_t limit =
        maximumCount < ordered.size() ? maximumCount : ordered.size();

    for (std::size_t i = 0; i < limit; ++i) {
        result.push_back(ordered[i].first);
    }
    return result;
}

} // namespace paras_advanced
