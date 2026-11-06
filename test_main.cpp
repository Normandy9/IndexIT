#include "DocumentStats.hpp"
#include "SearchAnalytics.hpp"
#include "ResultDeduplicator.hpp"
#include "RankingMetrics.hpp"
#include "PathUtilities.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

using namespace paras_advanced;

static void test_document_stats() {
    DocumentStats analyzer;
    DocumentStatistics stats = analyzer.analyze("alpha beta\n\nGamma");
    assert(stats.characters == 17);
    assert(stats.words == 3);
    assert(stats.lines == 3);
    assert(stats.nonEmptyLines == 2);
    assert(stats.averageWordLength > 4.0 && stats.averageWordLength < 5.0);
    DocumentStatistics empty = analyzer.analyze("");
    assert(empty.words == 0);
    assert(empty.averageWordLength == 0.0);
}

static void test_search_analytics() {
    std::vector<QueryEvent> events;
    QueryEvent a = {"machine", 3, 10}; events.push_back(a);
    QueryEvent b = {"python", 0, 20}; events.push_back(b);
    QueryEvent c = {"machine", 2, 30}; events.push_back(c);

    SearchAnalytics analytics;
    SearchSummary summary = analytics.summarize(events);
    assert(summary.totalQueries == 3);
    assert(summary.successfulQueries == 2);
    assert(summary.emptyQueries == 1);
    assert(summary.totalDurationMilliseconds == 60);
    assert(summary.averageDurationMilliseconds == 20.0);
    assert(summary.successRate > 66.6 && summary.successRate < 66.7);

    std::vector<std::string> top = analytics.mostFrequentQueries(events, 1);
    assert(top.size() == 1 && top[0] == "machine");
}

static void test_deduplicator() {
    std::vector<RankedPath> input;
    RankedPath a = {"D:\\Notes\\A.txt", 0.9}; input.push_back(a);
    RankedPath b = {"d:/notes/a.TXT", 0.5}; input.push_back(b);
    RankedPath c = {"D:/Notes/B.txt", 0.4}; input.push_back(c);

    ResultDeduplicator deduplicator;
    std::vector<RankedPath> output = deduplicator.removeDuplicatePaths(input);
    assert(output.size() == 2);
    assert(output[0].score == 0.9);
    assert(output[1].path == c.path);
}

static void test_ranking_metrics() {
    std::vector<std::string> retrieved;
    retrieved.push_back("a"); retrieved.push_back("b"); retrieved.push_back("x");
    std::vector<std::string> relevant;
    relevant.push_back("a"); relevant.push_back("b"); relevant.push_back("c");

    RankingMetrics metrics;
    RankingScore score = metrics.evaluate(retrieved, relevant);
    assert(std::fabs(score.precision - 2.0/3.0) < 0.00001);
    assert(std::fabs(score.recall - 2.0/3.0) < 0.00001);
    assert(std::fabs(score.f1 - 2.0/3.0) < 0.00001);
}

static void test_path_utilities() {
    assert(PathUtilities::normalizeSeparators("D:\\Docs\\a.txt") == "D:/Docs/a.txt");
    assert(PathUtilities::fileName("D:\\Docs\\a.txt") == "a.txt");
    assert(PathUtilities::extension("D:/Docs/a.txt") == ".txt");
    assert(PathUtilities::parentPath("D:/Docs/a.txt") == "D:/Docs");
    assert(PathUtilities::extension("README") == "");
}

int main() {
    test_document_stats();
    test_search_analytics();
    test_deduplicator();
    test_ranking_metrics();
    test_path_utilities();
    std::cout << "All advanced add-on tests passed.\n";
    return 0;
}
