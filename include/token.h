#ifndef TOKEN_H
#define TOKEN_H

#include <stdint.h>

typedef struct {
    char *text;
    uint32_t position;
} Token;

typedef struct {
    uint64_t document_id;
    uint32_t frequency;
} Posting;

#endif