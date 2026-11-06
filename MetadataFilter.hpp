#ifndef PARAS_PHASE3_METADATA_FILTER_HPP
#define PARAS_PHASE3_METADATA_FILTER_HPP

#include <string>
#include <vector>
#include <stdint.h>

namespace paras_phase3 {

struct FileMetadata {
    std::string path;
    std::string extension;
    uint64_t sizeBytes;
    int64_t modifiedEpoch;
};

struct MetadataFilter {
    std::string extension;
    std::string pathContains;
    uint64_t minimumBytes;
    uint64_t maximumBytes;
    int64_t modifiedAfter;
    int64_t modifiedBefore;
    bool useMinimum;
    bool useMaximum;
    bool useModifiedAfter;
    bool useModifiedBefore;

    MetadataFilter();
};

class MetadataFilterEngine {
public:
    std::vector<FileMetadata> apply(
        const std::vector<FileMetadata>& input,
        const MetadataFilter& filter) const;

    bool matches(const FileMetadata& item,
                 const MetadataFilter& filter) const;
};

} // namespace paras_phase3

#endif
