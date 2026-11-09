#pragma once

#include <algorithm>
#include <cstdint>
#include <cwctype>
#include <limits>
#include <string>

namespace indexit {

struct SearchFilters {
    std::wstring extension;
    std::wstring pathContains;
    uint64_t minSize = 0;
    uint64_t maxSize = (std::numeric_limits<uint64_t>::max)();

    bool matches(const std::wstring& path, uint64_t size) const {
        if (size < minSize || size > maxSize) return false;

        if (!extension.empty()) {
            const size_t slash = path.find_last_of(L"\\/");
            const size_t dot = path.find_last_of(L'.');
            if (dot == std::wstring::npos || (slash != std::wstring::npos && dot < slash)) return false;

            std::wstring expected = extension;
            if (expected.front() != L'.') expected.insert(expected.begin(), L'.');
            std::wstring actual = path.substr(dot);
            std::transform(expected.begin(), expected.end(), expected.begin(), towlower);
            std::transform(actual.begin(), actual.end(), actual.begin(), towlower);
            if (actual != expected) return false;
        }

        if (!pathContains.empty()) {
            std::wstring pathLower = path;
            std::wstring needleLower = pathContains;
            std::transform(pathLower.begin(), pathLower.end(), pathLower.begin(), towlower);
            std::transform(needleLower.begin(), needleLower.end(), needleLower.begin(), towlower);
            if (pathLower.find(needleLower) == std::wstring::npos) return false;
        }

        return true;
    }
};

} // namespace indexit
