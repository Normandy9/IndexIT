#ifndef PARAS_PHASE3_STRING_UTILITIES_HPP
#define PARAS_PHASE3_STRING_UTILITIES_HPP

#include <string>

namespace paras_phase3 {

std::string trim(const std::string& value);
std::string asciiLower(const std::string& value);
std::string normalizeExtension(const std::string& extension);
bool containsCaseInsensitive(const std::string& text,
                             const std::string& search);
std::string jsonEscape(const std::string& value);
std::string csvEscape(const std::string& value);

} // namespace paras_phase3

#endif
