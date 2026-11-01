#pragma once
#include <stdexcept>
#include <string>

namespace indexit {

class IndexItException : public std::runtime_error {
public:
    explicit IndexItException(const std::string& msg) : std::runtime_error(msg) {}
};

class IOException : public IndexItException {
public:
    explicit IOException(const std::string& msg) : IndexItException(msg) {}
};

class DatabaseException : public IndexItException {
public:
    explicit DatabaseException(const std::string& msg) : IndexItException(msg) {}
};

} // namespace indexit
