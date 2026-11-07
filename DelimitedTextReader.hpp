#ifndef PARAS_DELIMITED_TEXT_READER_HPP
#define PARAS_DELIMITED_TEXT_READER_HPP
#include <string>
#include <vector>
namespace paras {
struct DelimitedReadResult {
    bool success;
    std::vector<std::vector<std::string> > rows;
    std::string error;
};
class DelimitedTextReader {
public:
    static DelimitedReadResult parse(const std::string& content, char delimiter = ',');
    static std::string escapeField(const std::string& field, char delimiter = ',');
    static std::string joinRow(const std::vector<std::string>& fields,
                               char delimiter = ',');
};
}
#endif
