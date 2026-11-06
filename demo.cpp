#include "QueryParser.hpp"
#include "SnippetBuilder.hpp"
#include "ResultExporter.hpp"

#include <iostream>
#include <vector>

int main() {
    paras_phase3::QueryParser parser;
    paras_phase3::QueryPlan plan =
        parser.parse("machine learning -draft type:txt path:notes");

    if (!plan.valid()) {
        for (std::size_t i = 0; i < plan.errors.size(); ++i) {
            std::cerr << plan.errors[i] << "\n";
        }
        return 1;
    }

    std::cout << "Required terms: " << plan.requiredTerms.size() << "\n";
    std::cout << "Excluded terms: " << plan.excludedTerms.size() << "\n";
    std::cout << "Extension: " << plan.extension << "\n";
    std::cout << "Path contains: " << plan.pathContains << "\n";

    paras_phase3::SnippetBuilder builder;
    std::vector<std::string> terms;
    terms.push_back("learning");
    paras_phase3::Snippet preview =
        builder.build("IndexIt helps users search local documents. "
                      "Machine learning and data retrieval are useful topics.",
                      terms, 100);
    std::cout << "Preview: " << preview.text << "\n";
    return 0;
}
