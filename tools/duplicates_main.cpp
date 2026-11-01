#include <filesystem>
#include <iostream>
#include "DuplicateDetector.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: indexit_duplicates <directory>\n";
        return 2;
    }
    std::error_code ec;
    std::filesystem::path root = std::filesystem::u8path(argv[1]);
    if (!std::filesystem::is_directory(root, ec) || ec) {
        std::cerr << "Error: directory does not exist or cannot be accessed.\n";
        return 2;
    }
    auto groups = indexit::DuplicateDetector::find(root);
    if (groups.empty()) {
        std::cout << "No duplicate files found.\n";
        return 0;
    }
    std::cout << "Found " << groups.size() << " duplicate group(s):\n";
    for (std::size_t i = 0; i < groups.size(); ++i) {
        std::cout << "\nGroup " << (i + 1) << " | " << groups[i].files.size()
                  << " files | " << groups[i].fileSize << " bytes each\n";
        for (const auto& file : groups[i].files)
            std::cout << "  " << file.u8string() << "\n";
    }
    return 0;
}
