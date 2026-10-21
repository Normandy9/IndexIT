#ifndef TOKEN_H
#define TOKEN_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char *text;
    uint32_t position;
} Token;

typedef struct {
    uint64_t document_id;
    uint32_t frequency;
} Posting;

typedef struct Tokenizer Tokenizer;

Tokenizer* tokenizer_create(const char *text);
int tokenizer_next(Tokenizer *tokenizer, Token *out_token);
void tokenizer_destroy(Tokenizer *tokenizer);

#ifdef __cplusplus
}
#endif

#endif