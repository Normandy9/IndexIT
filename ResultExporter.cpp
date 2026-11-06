#include "ResultExporter.hpp"
#include "StringUtilities.hpp"

#include <sstream>
#include <iomanip>

namespace paras_phase3 {

std::string ResultExporter::toCsv(
    const std::vector<SearchResultRecord>& results) const {
    std::ostringstream out;
    out << "path,title,score,snippet\r\n";

    for (std::vector<SearchResultRecord>::const_iterator it = results.begin();
         it != results.end(); ++it) {
        out << csvEscape(it->path) << ','
            << csvEscape(it->title) << ','
            << std::setprecision(12) << it->score << ','
            << csvEscape(it->snippet) << "\r\n";
    }
    return out.str();
}

std::string ResultExporter::toJson(
    const std::vector<SearchResultRecord>& results) const {
    std::ostringstream out;
    out << "[";
    for (std::size_t i = 0; i < results.size(); ++i) {
        if (i != 0) out << ",";
        out << "\n  {"
            << "\"path\":\"" << jsonEscape(results[i].path) << "\","
            << "\"title\":\"" << jsonEscape(results[i].title) << "\","
            << "\"score\":" << std::setprecision(12) << results[i].score << ","
            << "\"snippet\":\"" << jsonEscape(results[i].snippet) << "\""
            << "}";
    }
    if (!results.empty()) out << "\n";
    out << "]";
    return out.str();
}

} // namespace paras_phase3
