#include "hashtable.h"
#include <stdlib.h>
#include <string.h>
static char* _my_strdup(const char* s) { char* d = malloc(strlen(s)+1); if(d) strcpy(d, s); return d; }

#define INITIAL_TABLE_SIZE 1024

typedef struct HashEntry {
    char *key;
    Posting *postings;
    size_t postings_count;
    size_t postings_capacity;
    struct HashEntry *next;
} HashEntry;

struct HashTable {
    HashEntry **buckets;
    size_t num_buckets;
};

// FNV-1a hash function
static size_t hash_func(const char *key, size_t num_buckets) {
    size_t hash = 2166136261u;
    while (*key) {
        hash ^= (unsigned char)(*key);
        hash *= 16777619;
        key++;
    }
    return hash % num_buckets;
}

HashTable* hash_create(void) {
    HashTable *table = (HashTable*)malloc(sizeof(HashTable));
    if (!table) return NULL;
    
    table->num_buckets = INITIAL_TABLE_SIZE;
    table->buckets = (HashEntry**)calloc(table->num_buckets, sizeof(HashEntry*));
    if (!table->buckets) {
        free(table);
        return NULL;
    }
    
    return table;
}

void hash_destroy(HashTable *table) {
    if (!table) return;
    
    for (size_t i = 0; i < table->num_buckets; i++) {
        HashEntry *entry = table->buckets[i];
        while (entry) {
            HashEntry *next = entry->next;
            free(entry->key);
            if (entry->postings) free(entry->postings);
            free(entry);
            entry = next;
        }
    }
    
    free(table->buckets);
    free(table);
}

int hash_insert(HashTable *table, const char *key, Posting posting) {
    if (!table || !key) return -1;
    
    size_t idx = hash_func(key, table->num_buckets);
    HashEntry *entry = table->buckets[idx];
    
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            // Found existing key, add posting
            for (size_t i = 0; i < entry->postings_count; i++) {
                if (entry->postings[i].document_id == posting.document_id) {
                    entry->postings[i].frequency += posting.frequency;
                    return 0;
                }
            }
            
            if (entry->postings_count >= entry->postings_capacity) {
                size_t new_cap = entry->postings_capacity == 0 ? 4 : entry->postings_capacity * 2;
                Posting *new_postings = (Posting*)realloc(entry->postings, new_cap * sizeof(Posting));
                if (!new_postings) return -1;
                entry->postings = new_postings;
                entry->postings_capacity = new_cap;
            }
            
            entry->postings[entry->postings_count++] = posting;
            return 0;
        }
        entry = entry->next;
    }
    
    // Key not found, create new entry
    HashEntry *new_entry = (HashEntry*)malloc(sizeof(HashEntry));
    if (!new_entry) return -1;
    
    new_entry->key = _my_strdup(key);
    new_entry->postings_capacity = 4;
    new_entry->postings = (Posting*)malloc(new_entry->postings_capacity * sizeof(Posting));
    if (!new_entry->postings) {
        free(new_entry->key);
        free(new_entry);
        return -1;
    }
    
    new_entry->postings[0] = posting;
    new_entry->postings_count = 1;
    
    new_entry->next = table->buckets[idx];
    table->buckets[idx] = new_entry;
    
    return 0;
}

Posting* hash_lookup(const HashTable *table, const char *key) {
    if (!table || !key) return NULL;
    
    size_t idx = hash_func(key, table->num_buckets);
    HashEntry *entry = table->buckets[idx];
    
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            return entry->postings;
        }
        entry = entry->next;
    }
    
    return NULL;
}

size_t hash_lookup_count(const HashTable *table, const char *key) {
    if (!table || !key) return 0;
    
    size_t idx = hash_func(key, table->num_buckets);
    HashEntry *entry = table->buckets[idx];
    
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            return entry->postings_count;
        }
        entry = entry->next;
    }
    
    return 0;
}
