#include "QueryValidator.hpp"
#include <cctype>
namespace paras {
ValidationResult::ValidationResult() : valid(true) {}
std::string QueryValidator::trim(const std::string& value) {
    std::size_t first = 0;
    std::size_t last = value.size();
    while (first < last &&
           std::isspace(static_cast<unsigned char>(value[first]))) {
        ++first;
    }
    while (last > first &&
           std::isspace(static_cast<unsigned char>(value[last - 1]))) {
        --last;
    }
    return value.substr(first, last - first);
}
bool QueryValidator::containsOnlyWhitespace(const std::string& value) {
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (!std::isspace(static_cast<unsigned char>(value[i]))) {
            return false;
        }
    }
    return true;
}
ValidationResult QueryValidator::validate(const std::string& query,
                                           std::size_t maximumLength) {
    ValidationResult result;
    if (query.empty()) {
        result.valid = false;
        result.issues.push_back({"EMPTY_QUERY", "The query is empty."});
        return result;
    }
    if (containsOnlyWhitespace(query)) {
        result.valid = false;
        result.issues.push_back({"BLANK_QUERY", "The query contains only spaces."});
    }
    if (query.size() > maximumLength) {
        result.valid = false;
        result.issues.push_back({"QUERY_TOO_LONG", "The query exceeds the length limit."});
    }
    for (std::size_t i = 0; i < query.size(); ++i) {
        unsigned char ch = static_cast<unsigned char>(query[i]);
        if (std::iscntrl(ch) && ch != '\t' && ch != '\n' && ch != '\r') {
            result.valid = false;
            result.issues.push_back({"CONTROL_CHARACTER", "The query contains a control character."});
            break;
        }
    }
    return result;
}
}
