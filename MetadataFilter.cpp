#include "MetadataFilter.hpp"
#include "StringUtilities.hpp"

namespace paras_phase3 {

MetadataFilter::MetadataFilter()
    : minimumBytes(0), maximumBytes(0),
      modifiedAfter(0), modifiedBefore(0),
      useMinimum(false), useMaximum(false),
      useModifiedAfter(false), useModifiedBefore(false) {}

bool MetadataFilterEngine::matches(const FileMetadata& item,
                                   const MetadataFilter& filter) const {
    if (!filter.extension.empty()) {
        const std::string wanted = normalizeExtension(filter.extension);
        const std::string actual = normalizeExtension(item.extension);
        if (wanted != actual) return false;
    }

    if (!filter.pathContains.empty() &&
        !containsCaseInsensitive(item.path, filter.pathContains)) {
        return false;
    }

    if (filter.useMinimum && item.sizeBytes < filter.minimumBytes) {
        return false;
    }

    if (filter.useMaximum && item.sizeBytes > filter.maximumBytes) {
        return false;
    }

    if (filter.useModifiedAfter &&
        item.modifiedEpoch < filter.modifiedAfter) {
        return false;
    }

    if (filter.useModifiedBefore &&
        item.modifiedEpoch > filter.modifiedBefore) {
        return false;
    }

    return true;
}

std::vector<FileMetadata> MetadataFilterEngine::apply(
    const std::vector<FileMetadata>& input,
    const MetadataFilter& filter) const {
    std::vector<FileMetadata> output;
    output.reserve(input.size());

    for (std::vector<FileMetadata>::const_iterator it = input.begin();
         it != input.end(); ++it) {
        if (matches(*it, filter)) output.push_back(*it);
    }
    return output;
}

} // namespace paras_phase3
