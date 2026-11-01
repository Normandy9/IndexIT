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
static char **dynamic_stopwords = NULL;
static int num_dynamic_stopwords = 0;

static int is_word_byte(unsigned char c) {
    /* Keep UTF-8 multibyte bytes together; full Unicode category handling is not available in C locale. */
    return c >= 0x80 || isalnum(c);
}

static const char *DEFAULT_STOPWORDS[] = {
    "a", "an", "and", "are", "as", "at", "be", "but", "by", "for", "if", "in", 
    "into", "is", "it", "no", "not", "of", "on", "or", "such", "that", "the", 
    "their", "then", "there", "these", "they", "this", "to", "was", "will", "with"
};
static const int NUM_DEFAULT_STOPWORDS = sizeof(DEFAULT_STOPWORDS) / sizeof(DEFAULT_STOPWORDS[0]);

void tokenizer_init_stopwords(const char *filepath) {
    if (!filepath) return;
    FILE *f = fopen(filepath, "r");
    if (!f) return;
    
    // Naive loading (max 1000 words, max 50 chars each)
    dynamic_stopwords = (char**)malloc(1000 * sizeof(char*));
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), f) && num_dynamic_stopwords < 1000) {
        size_t len = strlen(buffer);
        while (len > 0 && (buffer[len-1] == '\n' || buffer[len-1] == '\r')) {
            buffer[len-1] = '\0';
            len--;
        }
        if (len > 0) {
            char *copy = (char*)malloc(len + 1);
            if (copy) {
                memcpy(copy, buffer, len + 1);
                dynamic_stopwords[num_dynamic_stopwords++] = copy;
            }
        }
    }
    fclose(f);
}

static int is_stopword(const char *word) {
    if (dynamic_stopwords) {
        for (int i = 0; i < num_dynamic_stopwords; i++) {
            if (strcmp(word, dynamic_stopwords[i]) == 0) return 1;
        }
        return 0;
    }
    for (int i = 0; i < NUM_DEFAULT_STOPWORDS; i++) {
        if (strcmp(word, DEFAULT_STOPWORDS[i]) == 0) return 1;
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
        while (tokenizer->text[tokenizer->offset] != '\0' && !is_word_byte((unsigned char)tokenizer->text[tokenizer->offset])) {
            tokenizer->offset++;
        }
        
        if (tokenizer->text[tokenizer->offset] == '\0') break;
        
        // Mark start of token
        size_t start = tokenizer->offset;
        
        // Find end of token
        while (tokenizer->text[tokenizer->offset] != '\0' && is_word_byte((unsigned char)tokenizer->text[tokenizer->offset])) {
            tokenizer->offset++;
        }
        
        size_t len = tokenizer->offset - start;
        if (len > 0) {
            char *word = (char*)malloc(len + 1);
            if (!word) return 0;
            
            for (size_t i = 0; i < len; i++) {
                unsigned char c = (unsigned char)tokenizer->text[start + i];
                word[i] = c < 0x80 ? (char)tolower(c) : (char)c;
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
