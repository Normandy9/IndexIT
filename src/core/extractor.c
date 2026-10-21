#include "extractor.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper to read a whole text file into a newly allocated UTF-8 string.
static char* extract_plain_text(const wchar_t *filepath) {
    HANDLE hFile = CreateFileW(filepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return NULL;
    }
    
    LARGE_INTEGER size;
    if (!GetFileSizeEx(hFile, &size)) {
        CloseHandle(hFile);
        return NULL;
    }
    
    // Safety check: avoid loading files > 50MB for now
    if (size.QuadPart > 50 * 1024 * 1024) {
        CloseHandle(hFile);
        return NULL; 
    }
    
    size_t file_size = (size_t)size.QuadPart;
    char *buffer = (char *)malloc(file_size + 1);
    if (!buffer) {
        CloseHandle(hFile);
        return NULL;
    }
    
    DWORD bytes_read;
    if (!ReadFile(hFile, buffer, (DWORD)file_size, &bytes_read, NULL)) {
        free(buffer);
        CloseHandle(hFile);
        return NULL;
    }
    
    buffer[bytes_read] = '\0';
    CloseHandle(hFile);
    
    // A more robust implementation would check for UTF-16/BOM and convert,
    // but for Tier 1 we assume ANSI/UTF-8.
    return buffer;
}

void extractor_init(void) {
    // No-op for now
}

void extractor_cleanup(void) {
    // No-op for now
}

char* extract_text(const Document *doc) {
    if (!doc || !doc->path) return NULL;
    
    switch (doc->format) {
        case FORMAT_TXT:
        case FORMAT_CSV:
        case FORMAT_LOG:
        case FORMAT_MD:
        case FORMAT_JSON:
        case FORMAT_XML:
        case FORMAT_HTML:
        case FORMAT_C:
        case FORMAT_CPP:
        case FORMAT_H:
            return extract_plain_text(doc->path);
            
        case FORMAT_DOCX:
        case FORMAT_PPTX:
        case FORMAT_XLSX:
        case FORMAT_ODT:
        case FORMAT_ZIP:
            // TODO: Tier 2 (miniz)
            return NULL;
            
        case FORMAT_PDF:
            // TODO: Tier 3 (PDF)
            return NULL;
            
        default:
            return NULL;
    }
}
