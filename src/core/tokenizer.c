#include "token.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

struct Tokenizer {
    const char *text;
    size_t offset;
    uint32_t current_pos;
};

// Simple stopword list (binary search could be used if sorted, but linear is fine for a small list)
static const char *STOPWORDS[] = {
    "a", "an", "and", "are", "as", "at", "be", "but", "by", "for", "if", "in", 
    "into", "is", "it", "no", "not", "of", "on", "or", "such", "that", "the", 
    "their", "then", "there", "these", "they", "this", "to", "was", "will", "with"
};
static const int NUM_STOPWORDS = sizeof(STOPWORDS) / sizeof(STOPWORDS[0]);

static int is_stopword(const char *word) {
    for (int i = 0; i < NUM_STOPWORDS; i++) {
        if (strcmp(word, STOPWORDS[i]) == 0) return 1;
    }
    return 0;
}

Tokenizer* tokenizer_create(const char *text) {
    if (!text) return NULL;
    Tokenizer *t = (Tokenizer*)malloc(sizeof(Tokenizer));
    if (!t) return NULL;
    t->text = text;
    t->offset = 0;
    t->current_pos = 0;
    return t;
}

void tokenizer_destroy(Tokenizer *tokenizer) {
    if (tokenizer) {
        free(tokenizer);
    }
}

int tokenizer_next(Tokenizer *tokenizer, Token *out_token) {
    if (!tokenizer || !tokenizer->text || !out_token) return 0;
    
    while (tokenizer->text[tokenizer->offset] != '\0') {
        // Skip non-alphanumeric
        while (tokenizer->text[tokenizer->offset] != '\0' && !isalnum((unsigned char)tokenizer->text[tokenizer->offset])) {
            tokenizer->offset++;
        }
        
        if (tokenizer->text[tokenizer->offset] == '\0') break;
        
        // Mark start of token
        size_t start = tokenizer->offset;
        
        // Find end of token
        while (tokenizer->text[tokenizer->offset] != '\0' && isalnum((unsigned char)tokenizer->text[tokenizer->offset])) {
            tokenizer->offset++;
        }
        
        size_t len = tokenizer->offset - start;
        if (len > 0) {
            char *word = (char*)malloc(len + 1);
            if (!word) return 0;
            
            for (size_t i = 0; i < len; i++) {
                word[i] = (char)tolower((unsigned char)tokenizer->text[start + i]);
            }
            word[len] = '\0';
            
            tokenizer->current_pos++;
            
            if (is_stopword(word)) {
                free(word);
                continue; // Skip and look for next
            }
            
            out_token->text = word;
            out_token->position = tokenizer->current_pos;
            return 1;
        }
    }
    
    return 0;
}
