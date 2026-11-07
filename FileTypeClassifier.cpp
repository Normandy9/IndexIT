#include "FileTypeClassifier.hpp"
#include <cctype>
#include <map>
namespace paras {
namespace {
std::string lowerCase(std::string value) {
    for (std::size_t i = 0; i < value.size(); ++i) {
        value[i] = static_cast<char>(
            std::tolower(static_cast<unsigned char>(value[i])));
    }
    return value;
}
}
std::string FileTypeClassifier::extensionOf(const std::string& path) {
    std::size_t slash = path.find_last_of("/\\");
    std::size_t dot = path.find_last_of('.');
    if (dot == std::string::npos) return "";
    if (slash != std::string::npos && dot < slash) return "";
    return lowerCase(path.substr(dot));
}
FileTypeInfo FileTypeClassifier::classify(const std::string& path) {
    static const std::map<std::string, FileTypeInfo> types = {
        {".txt", {".txt", "text", "Plain text document", true}},
        {".md", {".md", "text", "Markdown document", true}},
        {".csv", {".csv", "data", "Comma separated values", true}},
        {".json", {".json", "data", "JSON document", true}},
        {".xml", {".xml", "data", "XML document", true}},
        {".html", {".html", "web", "HTML document", true}},
        {".htm", {".htm", "web", "HTML document", true}},
        {".css", {".css", "web", "CSS stylesheet", true}},
        {".js", {".js", "code", "JavaScript source", true}},
        {".c", {".c", "code", "C source file", true}},
        {".cpp", {".cpp", "code", "C++ source file", true}},
        {".h", {".h", "code", "C or C++ header", true}},
        {".hpp", {".hpp", "code", "C++ header", true}},
        {".py", {".py", "code", "Python source file", true}},
        {".java", {".java", "code", "Java source file", true}},
        {".pdf", {".pdf", "document", "PDF document", false}},
        {".docx", {".docx", "document", "Word document", false}},
        {".xlsx", {".xlsx", "spreadsheet", "Excel workbook", false}},
        {".pptx", {".pptx", "presentation", "PowerPoint file", false}},
        {".png", {".png", "image", "PNG image", false}},
        {".jpg", {".jpg", "image", "JPEG image", false}},
        {".jpeg", {".jpeg", "image", "JPEG image", false}},
        {".gif", {".gif", "image", "GIF image", false}},
        {".mp3", {".mp3", "audio", "MP3 audio", false}},
        {".mp4", {".mp4", "video", "MP4 video", false}}
    };
    std::string extension = extensionOf(path);
    std::map<std::string, FileTypeInfo>::const_iterator found = types.find(extension);
    if (found != types.end()) return found->second;
    return {extension, "unknown", "Unclassified file type", false};
}
std::vector<std::string> FileTypeClassifier::supportedExtensions() {
    return {".txt", ".md", ".csv", ".json", ".xml", ".html", ".htm",
            ".css", ".js", ".c", ".cpp", ".h", ".hpp", ".py", ".java",
            ".pdf", ".docx", ".xlsx", ".pptx", ".png", ".jpg", ".jpeg",
            ".gif", ".mp3", ".mp4"};
}
bool FileTypeClassifier::isTextLike(const std::string& path) {
    return classify(path).textLike;
}
}
