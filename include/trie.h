#ifndef TRIE_H
#define TRIE_H

#include "token.h"

typedef struct Trie Trie;

Trie *trie_create(void);
void trie_destroy(Trie *trie);

int trie_insert(Trie *trie, const char *term, Posting posting);
Posting *trie_search(const Trie *trie, const char *term);

#endif