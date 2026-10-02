#pragma once
#include <string>
#include <memory>
#include "document.h"

namespace indexit {

class IExtractor {
public:
    virtual ~IExtractor() = default;
    virtual std::string extract(const std::wstring& filepath) = 0;
};

class PlainExtractor : public IExtractor {
public:
    std::string extract(const std::wstring& filepath) override;
};

class DocxExtractor : public IExtractor {
public:
    std::string extract(const std::wstring& filepath) override;
};

class PdfExtractor : public IExtractor {
public:
    std::string extract(const std::wstring& filepath) override;
};

class ZipExtractor : public IExtractor {
public:
    std::string extract(const std::wstring& filepath) override;
};

class ExtractorFactory {
public:
    static std::unique_ptr<IExtractor> create(DocumentFormat format);
};

} // namespace indexit
