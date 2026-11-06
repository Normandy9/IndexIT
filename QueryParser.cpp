#include "QueryParser.hpp"
#include "StringUtilities.hpp"

namespace paras_phase3 {

bool QueryPlan::valid() const {
    return errors.empty();
}

bool QueryPlan::empty() const {
    return requiredTerms.empty() && excludedTerms.empty() &&
           phrases.empty() && extension.empty() && pathContains.empty();
}

bool QueryParser::isFieldToken(const std::string& token) {
    return token.find(':') != std::string::npos;
}

QueryPlan QueryParser::parse(const std::string& query) const {
    QueryPlan plan;
    std::string token;
    bool inQuote = false;
    bool tokenStartedWithMinus = false;

    const std::string cleaned = trim(query);
    if (cleaned.empty()) {
        plan.errors.push_back("Query is empty.");
        return plan;
    }

    for (std::string::size_type i = 0; i < cleaned.size(); ++i) {
        const char ch = cleaned[i];

        if (ch == '"') {
            if (!inQuote) {
                if (!token.empty()) {
                    plan.errors.push_back(
                        "A quoted phrase must start at a token boundary.");
                    return plan;
                }
                inQuote = true;
            } else {
                const std::string phrase = trim(token);
                if (phrase.empty()) {
                    plan.errors.push_back("Empty quoted phrase is not allowed.");
                    return plan;
                }
                plan.phrases.push_back(phrase);
                token.clear();
                inQuote = false;
                tokenStartedWithMinus = false;
            }
            continue;
        }

        if (!inQuote && (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')) {
            if (!token.empty()) {
                if (tokenStartedWithMinus) {
                    if (token.size() == 1) {
                        plan.errors.push_back("A minus sign must exclude a term.");
                        return plan;
                    }
                    plan.excludedTerms.push_back(asciiLower(token.substr(1)));
                } else if (token.compare(0, 5, "type:") == 0) {
                    if (token.size() == 5) {
                        plan.errors.push_back("type: requires an extension.");
                        return plan;
                    }
                    if (!plan.extension.empty()) {
                        plan.errors.push_back("Only one type: filter is supported.");
                        return plan;
                    }
                    plan.extension = normalizeExtension(token.substr(5));
                } else if (token.compare(0, 5, "path:") == 0) {
                    if (token.size() == 5) {
                        plan.errors.push_back("path: requires a value.");
                        return plan;
                    }
                    if (!plan.pathContains.empty()) {
                        plan.errors.push_back("Only one path: filter is supported.");
                        return plan;
                    }
                    plan.pathContains = token.substr(5);
                } else if (isFieldToken(token)) {
                    plan.errors.push_back("Unknown query field: " + token);
                    return plan;
                } else {
                    plan.requiredTerms.push_back(asciiLower(token));
                }
                token.clear();
                tokenStartedWithMinus = false;
            }
            continue;
        }

        if (!inQuote && token.empty() && ch == '-') {
            tokenStartedWithMinus = true;
        }
        token += ch;
    }

    if (inQuote) {
        plan.errors.push_back("Unclosed quotation mark.");
        return plan;
    }

    if (!token.empty()) {
        if (tokenStartedWithMinus) {
            if (token.size() == 1) {
                plan.errors.push_back("A minus sign must exclude a term.");
                return plan;
            }
            plan.excludedTerms.push_back(asciiLower(token.substr(1)));
        } else if (token.compare(0, 5, "type:") == 0) {
            if (token.size() == 5) {
                plan.errors.push_back("type: requires an extension.");
            } else if (!plan.extension.empty()) {
                plan.errors.push_back("Only one type: filter is supported.");
            } else {
                plan.extension = normalizeExtension(token.substr(5));
            }
        } else if (token.compare(0, 5, "path:") == 0) {
            if (token.size() == 5) {
                plan.errors.push_back("path: requires a value.");
            } else if (!plan.pathContains.empty()) {
                plan.errors.push_back("Only one path: filter is supported.");
            } else {
                plan.pathContains = token.substr(5);
            }
        } else if (isFieldToken(token)) {
            plan.errors.push_back("Unknown query field: " + token);
        } else {
            plan.requiredTerms.push_back(asciiLower(token));
        }
    }

    if (plan.valid() && plan.empty()) {
        plan.errors.push_back("Query does not contain a searchable term or filter.");
    }

    return plan;
}

} // namespace paras_phase3
