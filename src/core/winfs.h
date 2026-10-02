#ifndef WINFS_H
#define WINFS_H

#include "document.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*FileDiscoveryCallback)(const Document *doc, void *user_data);

int winfs_walk_directory(const wchar_t *path, FileDiscoveryCallback callback, void *user_data);

#ifdef __cplusplus
}
#endif

#endif
