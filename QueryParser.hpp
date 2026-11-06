#ifndef PARAS_PHASE3_QUERY_PARSER_HPP
#define PARAS_PHASE3_QUERY_PARSER_HPP

#include <string>
#include <vector>

namespace paras_phase3 {

struct QueryPlan {
    std::vector<std::string> requiredTerms;
    std::vector<std::string> excludedTerms;
    std::vector<std::string> phrases;
    std::string extension;
    std::string pathContains;
    std::vector<std::string> errors;

    bool valid() const;
    bool empty() const;
};

class QueryParser {
public:
    QueryPlan parse(const std::string& query) const;

private:
    static bool isFieldToken(const std::string& token);
};

} // namespace paras_phase3

#endif
