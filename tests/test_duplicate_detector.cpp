#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "DuplicateDetector.hpp"

int main() {
    namespace fs = std::filesystem;
    fs::path root = fs::temp_directory_path() / "indexit_duplicate_detector_test";
    std::error_code ec;
    fs::remove_all(root, ec);
    fs::create_directories(root / "nested");
    { std::ofstream f(root / "one.txt", std::ios::binary); f << "same content\n"; }
    { std::ofstream f(root / "nested" / "two.txt", std::ios::binary); f << "same content\n"; }
    { std::ofstream f(root / "different.txt", std::ios::binary); f << "different content\n"; }
    { std::ofstream f(root / "empty-a.txt", std::ios::binary); }
    { std::ofstream f(root / "empty-b.txt", std::ios::binary); }

    auto groups = indexit::DuplicateDetector::find(root);
    assert(groups.size() == 2);
    bool foundPair = false, foundEmptyPair = false;
    for (const auto& group : groups) {
        assert(group.files.size() == 2);
        if (group.fileSize == 13) foundPair = true;
        if (group.fileSize == 0) foundEmptyPair = true;
    }
    assert(foundPair && foundEmptyPair);
    fs::remove_all(root, ec);
    std::cout << "Duplicate detector tests passed.\n";
    return 0;
}
