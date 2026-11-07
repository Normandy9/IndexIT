#include "QueryValidator.hpp"
#include "FileTypeClassifier.hpp"
#include "SearchSuggestionEngine.hpp"
#include "DelimitedTextReader.hpp"
#include <iostream>
int main() {
    using namespace paras;
    ValidationResult validation = QueryValidator::validate("machine learning");
    std::cout << "Query valid: " << (validation.valid ? "yes" : "no") << "\n";
    FileTypeInfo type = FileTypeClassifier::classify("research.pdf");
    std::cout << "File category: " << type.category << "\n";
    SearchSuggestionEngine suggestions;
    suggestions.add("machine", 8);
    suggestions.add("management", 3);
    suggestions.add("manual", 2);
    std::vector<Suggestion> matches = suggestions.suggest("ma");
    for (std::size_t i = 0; i < matches.size(); ++i) {
        std::cout << matches[i].term << " (" << matches[i].frequency << ")\n";
    }
    DelimitedReadResult rows = DelimitedTextReader::parse("id,title\n1,IndexIt\n");
    std::cout << "CSV rows: " << rows.rows.size() << "\n";
    return 0;
}
