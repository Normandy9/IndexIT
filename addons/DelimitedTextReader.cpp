#include "DelimitedTextReader.hpp"
namespace paras {
DelimitedReadResult DelimitedTextReader::parse(const std::string& content,
                                                char delimiter) {
    DelimitedReadResult result;
    result.success = true;
    bool quoted = false;
    bool fieldStarted = false;
    std::string field;
    std::vector<std::string> row;
    for (std::size_t i = 0; i < content.size(); ++i) {
        char ch = content[i];
        if (quoted) {
            if (ch == '"') {
                if (i + 1 < content.size() && content[i + 1] == '"') {
                    field += '"';
                    ++i;
                } else {
                    quoted = false;
                }
            } else {
                field += ch;
            }
        } else if (ch == '"' && !fieldStarted) {
            quoted = true;
            fieldStarted = true;
        } else if (ch == delimiter) {
            row.push_back(field);
            field.clear();
            fieldStarted = false;
        } else if (ch == '\n' || ch == '\r') {
            if (ch == '\r' && i + 1 < content.size() && content[i + 1] == '\n') {
                ++i;
            }
            row.push_back(field);
            result.rows.push_back(row);
            row.clear();
            field.clear();
            fieldStarted = false;
        } else {
            field += ch;
            fieldStarted = true;
        }
    }
    if (quoted) {
        result.success = false;
        result.error = "Unclosed quoted field.";
        result.rows.clear();
        return result;
    }
    if (!field.empty() || !row.empty() || fieldStarted) {
        row.push_back(field);
        result.rows.push_back(row);
    }
    return result;
}
std::string DelimitedTextReader::escapeField(const std::string& field,
                                              char delimiter) {
    bool needsQuotes = field.find(delimiter) != std::string::npos ||
                       field.find('"') != std::string::npos ||
                       field.find('\n') != std::string::npos ||
                       field.find('\r') != std::string::npos;
    if (!needsQuotes) return field;
    std::string output = "\"";
    for (std::size_t i = 0; i < field.size(); ++i) {
        if (field[i] == '"') output += "\"\"";
        else output += field[i];
    }
    output += "\"";
    return output;
}
std::string DelimitedTextReader::joinRow(const std::vector<std::string>& fields,
                                          char delimiter) {
    std::string output;
    for (std::size_t i = 0; i < fields.size(); ++i) {
        if (i != 0) output += delimiter;
        output += escapeField(fields[i], delimiter);
    }
    return output;
}
}
