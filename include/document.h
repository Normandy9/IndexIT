#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <stdint.h>
#include <time.h>

typedef enum {
    FORMAT_UNKNOWN = 0,
    FORMAT_TXT,
    FORMAT_DOCX,
    FORMAT_PPTX,
    FORMAT_XLSX,
    FORMAT_PDF,
    FORMAT_ZIP
} DocumentFormat;

typedef struct {
    uint64_t id;
    char *path;
    uint64_t size;
    time_t timestamp;
    DocumentFormat format;
} Document;

#endif