#include "StringUtilities.hpp"

#include <cctype>

namespace paras_phase3 {

std::string trim(const std::string& value) {
    std::string::size_type first = 0;
    while (first < value.size() &&
           std::isspace(static_cast<unsigned char>(value[first]))) {
        ++first;
    }

    std::string::size_type last = value.size();
    while (last > first &&
           std::isspace(static_cast<unsigned char>(value[last - 1]))) {
        --last;
    }

    return value.substr(first, last - first);
}

std::string asciiLower(const std::string& value) {
    std::string result = value;
    for (std::string::size_type i = 0; i < result.size(); ++i) {
        unsigned char ch = static_cast<unsigned char>(result[i]);
        if (ch >= 'A' && ch <= 'Z') {
            result[i] = static_cast<char>(ch - 'A' + 'a');
        }
    }
    return result;
}

std::string normalizeExtension(const std::string& extension) {
    std::string result = asciiLower(trim(extension));
    if (result.empty()) return result;
    if (result[0] != '.') result.insert(result.begin(), '.');
    return result;
}

bool containsCaseInsensitive(const std::string& text,
                             const std::string& search) {
    if (search.empty()) return true;
    const std::string haystack = asciiLower(text);
    const std::string needle = asciiLower(search);
    return haystack.find(needle) != std::string::npos;
}

std::string jsonEscape(const std::string& value) {
    std::string out;
    out.reserve(value.size() + 8);

    for (std::string::size_type i = 0; i < value.size(); ++i) {
        const unsigned char ch = static_cast<unsigned char>(value[i]);
        switch (ch) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (ch < 0x20) {
                    static const char hex[] = "0123456789abcdef";
                    out += "\\u00";
                    out += hex[(ch >> 4) & 0x0f];
                    out += hex[ch & 0x0f];
                } else {
                    out += static_cast<char>(ch);
                }
        }
    }
    return out;
}

std::string csvEscape(const std::string& value) {
    bool needsQuotes = false;
    for (std::string::size_type i = 0; i < value.size(); ++i) {
        if (value[i] == ',' || value[i] == '"' ||
            value[i] == '\n' || value[i] == '\r') {
            needsQuotes = true;
            break;
        }
    }

    if (!needsQuotes) return value;

    std::string out = "\"";
    for (std::string::size_type i = 0; i < value.size(); ++i) {
        if (value[i] == '"') out += "\"\"";
        else out += value[i];
    }
    out += "\"";
    return out;
}

} // namespace paras_phase3
