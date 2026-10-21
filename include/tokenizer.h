#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "token.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Tokenizer Tokenizer;

Tokenizer* tokenizer_create(const char *text);
int tokenizer_next(Tokenizer *tokenizer, Token *out_token);
void tokenizer_destroy(Tokenizer *tokenizer);

#ifdef __cplusplus
}
#endif

#endif
