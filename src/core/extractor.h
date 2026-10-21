#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#include "document.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef char* (*ExtractTextFunc)(const wchar_t *filepath);

void extractor_init(void);
void extractor_cleanup(void);
char* extract_text(const Document *doc);

#ifdef __cplusplus
}
#endif

#endif
