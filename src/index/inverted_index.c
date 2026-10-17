#include "inverted_index.h"
#include "hashtable.h"
#include "trie.h"
#include <stdlib.h>

struct InvertedIndex {
    HashTable *hash;
    Trie *trie;
};

InvertedIndex* index_create(void) {
    InvertedIndex *idx = (InvertedIndex*)malloc(sizeof(InvertedIndex));
    if (!idx) return NULL;
    
    idx->hash = hash_create();
    idx->trie = trie_create();
    
    if (!idx->hash || !idx->trie) {
        index_destroy(idx);
        return NULL;
    }
    
    return idx;
}

void index_destroy(InvertedIndex *index) {
    if (index) {
        if (index->hash) hash_destroy(index->hash);
        if (index->trie) trie_destroy(index->trie);
        free(index);
    }
}

int index_insert(InvertedIndex *index, const char *term, Posting posting) {
    if (!index || !term) return -1;
    
    int r1 = hash_insert(index->hash, term, posting);
    int r2 = trie_insert(index->trie, term, posting);
    
    return (r1 == 0 && r2 == 0) ? 0 : -1;
}

Posting* index_lookup(const InvertedIndex *index, const char *term) {
    if (!index || !term) return NULL;
    // Default exact lookup via Hash
    return hash_lookup(index->hash, term);
}

size_t index_lookup_count(const InvertedIndex *index, const char *term) {
    if (!index || !term) return 0;
    return hash_lookup_count(index->hash, term);
}
