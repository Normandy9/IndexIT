#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "token.h"

typedef struct HashTable HashTable;

HashTable *hash_create(void);
void hash_destroy(HashTable *table);

int hash_insert(HashTable *table, const char *key, Posting posting);
Posting *hash_lookup(const HashTable *table, const char *key);

#endif