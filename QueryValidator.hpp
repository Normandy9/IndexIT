#ifndef PARAS_QUERY_VALIDATOR_HPP
#define PARAS_QUERY_VALIDATOR_HPP
#include <string>
#include <vector>
namespace paras {
struct ValidationIssue {
    std::string code;
    std::string message;
};
struct ValidationResult {
    bool valid;
    std::vector<ValidationIssue> issues;
    ValidationResult();
};
class QueryValidator {
public:
    static ValidationResult validate(const std::string& query,
                                     std::size_t maximumLength = 256);
    static std::string trim(const std::string& value);
    static bool containsOnlyWhitespace(const std::string& value);
};
}
#endif
