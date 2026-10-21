#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "token.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HashTable HashTable;

HashTable *hash_create(void);
void hash_destroy(HashTable *table);
int hash_insert(HashTable *table, const char *key, Posting posting);
Posting *hash_lookup(const HashTable *table, const char *key);
size_t hash_lookup_count(const HashTable *table, const char *key);

double hash_insert_timed(HashTable *table, const char *key, Posting posting);
double hash_lookup_timed(const HashTable *table, const char *key, Posting **out_postings, size_t *out_count);

#ifdef __cplusplus
}
#endif

#endif
