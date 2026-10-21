#include "IExtractor.hpp"
#include "RAII.hpp"
#include <fstream>
#include <sstream>

namespace indexit {

std::string PlainExtractor::extract(const std::wstring& filepath) {
    try {
        MappedFile mf(filepath);
        // Skip files larger than 50MB to prevent std::bad_alloc crashes
        if (mf.length() > 50 * 1024 * 1024) {
            return "";
        }
        return std::string(mf.data(), mf.length());
    } catch (const IOException&) {
        return "";
    } catch (const std::exception&) {
        return ""; // Catch bad_alloc or other STL exceptions safely
    }
}

std::string DocxExtractor::extract(const std::wstring& filepath) {
    // Stubbed out miniz parsing for now
    return "docx_content_stub";
}

std::string PdfExtractor::extract(const std::wstring& filepath) {
    // Stubbed out PDFium/MuPDF
    return "pdf_content_stub";
}

std::string ZipExtractor::extract(const std::wstring& filepath) {
    return "zip_content_stub";
}

std::unique_ptr<IExtractor> ExtractorFactory::create(DocumentFormat format) {
    switch (format) {
        case FORMAT_TXT:
        case FORMAT_CSV:
        case FORMAT_MD:
        case FORMAT_LOG:
        case FORMAT_C:
        case FORMAT_CPP:
        case FORMAT_H:
            return std::make_unique<PlainExtractor>();
        case FORMAT_DOCX:
            return std::make_unique<DocxExtractor>();
        case FORMAT_PDF:
            return std::make_unique<PdfExtractor>();
        case FORMAT_ZIP:
            return std::make_unique<ZipExtractor>();
        default:
            return nullptr;
    }
}

} // namespace indexit
