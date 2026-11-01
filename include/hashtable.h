#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "token.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HashTable HashTable;

HashTable *hash_create(void);
HashTable *hash_create_with_capacity(size_t initial_buckets);
void hash_destroy(HashTable *table);

int hash_insert(HashTable *table, const char *key, Posting posting);
Posting *hash_lookup(const HashTable *table, const char *key);
size_t hash_lookup_count(const HashTable *table, const char *key);

typedef void (*HashEntryVisitor)(const char *key, const Posting *postings, size_t count, void *user_data);
void hash_for_each(const HashTable *table, HashEntryVisitor visitor, void *user_data);

size_t hash_size(const HashTable *table);
size_t hash_num_buckets(const HashTable *table);
double hash_load_factor(const HashTable *table);

double hash_insert_timed(HashTable *table, const char *key, Posting posting);
double hash_lookup_timed(const HashTable *table, const char *key, Posting **out_postings, size_t *out_count);

#ifdef __cplusplus
}
#endif

#endif
