#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <stdint.h>
#include <time.h>
#include <wchar.h>

typedef enum {
    FORMAT_UNKNOWN = 0,
    FORMAT_TXT,
    FORMAT_CSV,
    FORMAT_LOG,
    FORMAT_MD,
    FORMAT_JSON,
    FORMAT_XML,
    FORMAT_HTML,
    FORMAT_C,
    FORMAT_CPP,
    FORMAT_H,
    FORMAT_DOCX,
    FORMAT_PPTX,
    FORMAT_XLSX,
    FORMAT_ODT,
    FORMAT_PDF,
    FORMAT_ZIP
} DocumentFormat;

typedef struct {
    uint64_t id;
    wchar_t *path;
    uint64_t size;
    time_t timestamp;
    DocumentFormat format;
} Document;

#endif