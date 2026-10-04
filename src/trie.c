#include "trie.h"
#include <stdlib.h>
#include <string.h>

struct Trie {
    int placeholder;
};

Trie *trie_create(void)
{
    Trie *trie = malloc(sizeof(Trie));
    if (trie == NULL)
        return NULL;

    trie->placeholder = 0;
    return trie;
}

void trie_destroy(Trie *trie)
{
    free(trie);
}

int trie_insert(Trie *trie, const char *term, Posting posting)
{
    (void)trie;
    (void)term;
    (void)posting;

    return 0;
}

Posting *trie_search(const Trie *trie, const char *term)
{
    (void)trie;
    (void)term;

    return NULL;
}