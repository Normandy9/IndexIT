#ifndef PARAS_ADVANCED_PATH_UTILITIES_HPP
#define PARAS_ADVANCED_PATH_UTILITIES_HPP

#include <string>

namespace paras_advanced {

class PathUtilities {
public:
    static std::string normalizeSeparators(const std::string& path);
    static std::string fileName(const std::string& path);
    static std::string extension(const std::string& path);
    static std::string parentPath(const std::string& path);
};

} // namespace paras_advanced

#endif
