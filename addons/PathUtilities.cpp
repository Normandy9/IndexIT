#include "PathUtilities.hpp"
#include <algorithm>

namespace paras_advanced {

std::string PathUtilities::normalizeSeparators(const std::string& path) {
    std::string result = path;
    std::replace(result.begin(), result.end(), '\\', '/');
    return result;
}

std::string PathUtilities::fileName(const std::string& path) {
    const std::string normalized = normalizeSeparators(path);
    const std::string::size_type position = normalized.find_last_of('/');
    if (position == std::string::npos) return normalized;
    return normalized.substr(position + 1);
}

std::string PathUtilities::extension(const std::string& path) {
    const std::string name = fileName(path);
    const std::string::size_type position = name.find_last_of('.');
    if (position == std::string::npos || position == 0) return "";
    return name.substr(position);
}

std::string PathUtilities::parentPath(const std::string& path) {
    const std::string normalized = normalizeSeparators(path);
    const std::string::size_type position = normalized.find_last_of('/');
    if (position == std::string::npos) return "";
    if (position == 0) return "/";
    return normalized.substr(0, position);
}

} // namespace paras_advanced
