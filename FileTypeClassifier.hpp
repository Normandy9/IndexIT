#ifndef PARAS_FILE_TYPE_CLASSIFIER_HPP
#define PARAS_FILE_TYPE_CLASSIFIER_HPP
#include <string>
#include <vector>
namespace paras {
struct FileTypeInfo {
    std::string extension;
    std::string category;
    std::string description;
    bool textLike;
};
class FileTypeClassifier {
public:
    static std::string extensionOf(const std::string& path);
    static FileTypeInfo classify(const std::string& path);
    static std::vector<std::string> supportedExtensions();
    static bool isTextLike(const std::string& path);
};
}
#endif
