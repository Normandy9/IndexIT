#include <wchar.h>
#include "winfs.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static DocumentFormat get_format_from_ext(const wchar_t *ext) {
    if (!ext) return FORMAT_UNKNOWN;
    if (_wcsicmp(ext, L".txt") == 0) return FORMAT_TXT;
    if (_wcsicmp(ext, L".csv") == 0) return FORMAT_CSV;
    if (_wcsicmp(ext, L".log") == 0) return FORMAT_LOG;
    if (_wcsicmp(ext, L".md") == 0) return FORMAT_MD;
    if (_wcsicmp(ext, L".json") == 0) return FORMAT_JSON;
    if (_wcsicmp(ext, L".xml") == 0) return FORMAT_XML;
    if (_wcsicmp(ext, L".html") == 0) return FORMAT_HTML;
    if (_wcsicmp(ext, L".c") == 0) return FORMAT_C;
    if (_wcsicmp(ext, L".cpp") == 0) return FORMAT_CPP;
    if (_wcsicmp(ext, L".h") == 0) return FORMAT_H;
    if (_wcsicmp(ext, L".docx") == 0) return FORMAT_DOCX;
    if (_wcsicmp(ext, L".pptx") == 0) return FORMAT_PPTX;
    if (_wcsicmp(ext, L".xlsx") == 0) return FORMAT_XLSX;
    if (_wcsicmp(ext, L".odt") == 0) return FORMAT_ODT;
    if (_wcsicmp(ext, L".pdf") == 0) return FORMAT_PDF;
    if (_wcsicmp(ext, L".zip") == 0) return FORMAT_ZIP;
    return FORMAT_UNKNOWN;
}

static time_t filetime_to_time_t(const FILETIME *ft) {
    ULARGE_INTEGER ull;
    ull.LowPart = ft->dwLowDateTime;
    ull.HighPart = ft->dwHighDateTime;
    return (time_t)((ull.QuadPart / 10000000ULL) - 11644473600ULL);
}

int winfs_walk_directory(const wchar_t *path, FileDiscoveryCallback callback, void *user_data) {
    if (!path || !callback) return -1;
    
    wchar_t search_path[MAX_PATH * 4];
    if (wcslen(path) + 3 >= MAX_PATH * 4) return -1; // Path too long
    
    _snwprintf(search_path, MAX_PATH * 4, L"%ls\\*", path);
    
    WIN32_FIND_DATAW find_data;
    HANDLE hFind = FindFirstFileW(search_path, &find_data);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        if (err == ERROR_ACCESS_DENIED) {
            wprintf(L"WARN: Access denied to directory: %s\n", path);
        }
        return 0; // Skip unreadable directories
    }
    
    int result = 0;
    
    do {
        if (wcscmp(find_data.cFileName, L".") == 0 || wcscmp(find_data.cFileName, L"..") == 0) {
            continue;
        }
        
        // Skip hidden and system files
        if (find_data.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)) {
            continue;
        }
        
        wchar_t full_path[MAX_PATH * 4];
        _snwprintf(full_path, MAX_PATH * 4, L"%ls\\%ls", path, find_data.cFileName);
        
        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // Recurse
            result = winfs_walk_directory(full_path, callback, user_data);
            if (result != 0) break;
        } else {
            // We found a file
            // Attempt to open it briefly to check for sharing violation
            HANDLE hFile = CreateFileW(full_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile == INVALID_HANDLE_VALUE) {
                DWORD err = GetLastError();
                if (err == ERROR_SHARING_VIOLATION) {
                    wprintf(L"WARN: File locked, skipping: %s\n", full_path);
                    continue;
                } else if (err == ERROR_ACCESS_DENIED) {
                    wprintf(L"WARN: Access denied, skipping: %s\n", full_path);
                    continue;
                }
            } else {
                CloseHandle(hFile);
            }
            
            Document doc;
            doc.id = 0; // To be assigned by DB
            doc.path = full_path;
            doc.size = ((uint64_t)find_data.nFileSizeHigh << 32) | find_data.nFileSizeLow;
            doc.timestamp = filetime_to_time_t(&find_data.ftLastWriteTime);
            
            const wchar_t *ext = wcsrchr(find_data.cFileName, L'.');
            doc.format = get_format_from_ext(ext);
            
            result = callback(&doc, user_data);
            if (result != 0) break;
        }
    } while (FindNextFileW(hFind, &find_data) != 0);
    
    FindClose(hFind);
    return result;
}
