#ifndef PARAS_PHASE3_RESULT_EXPORTER_HPP
#define PARAS_PHASE3_RESULT_EXPORTER_HPP

#include <string>
#include <vector>

namespace paras_phase3 {

struct SearchResultRecord {
    std::string path;
    std::string title;
    double score;
    std::string snippet;
};

class ResultExporter {
public:
    std::string toCsv(const std::vector<SearchResultRecord>& results) const;
    std::string toJson(const std::vector<SearchResultRecord>& results) const;
};

} // namespace paras_phase3

#endif
