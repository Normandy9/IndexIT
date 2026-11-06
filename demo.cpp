#include "DocumentStats.hpp"
#include "RankingMetrics.hpp"
#include "SearchAnalytics.hpp"

#include <iostream>
#include <vector>

int main() {
    paras_advanced::DocumentStats analyzer;
    paras_advanced::DocumentStatistics stats =
        analyzer.analyze("IndexIt searches local documents.\nIt ranks useful results.");

    std::cout << "Words: " << stats.words << "\n";
    std::cout << "Lines: " << stats.lines << "\n";
    std::cout << "Average word length: " << stats.averageWordLength << "\n";

    std::vector<std::string> returned;
    returned.push_back("doc1");
    returned.push_back("doc2");
    std::vector<std::string> relevant;
    relevant.push_back("doc1");
    relevant.push_back("doc3");

    paras_advanced::RankingMetrics metrics;
    paras_advanced::RankingScore score = metrics.evaluate(returned, relevant);
    std::cout << "Precision: " << score.precision << "\n";
    std::cout << "Recall: " << score.recall << "\n";
    std::cout << "F1: " << score.f1 << "\n";
    return 0;
}
