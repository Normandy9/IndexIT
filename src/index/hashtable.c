#include "hashtable.h"
#include <stdlib.h>
#include <string.h>
#include <windows.h>

static char* _my_strdup(const char* s) {
    if (!s) return NULL;
    char* d = (char*)malloc(strlen(s) + 1);
    if (d) strcpy(d, s);
    return d;
}

#define DEFAULT_INITIAL_BUCKETS 1024
#define MAX_LOAD_FACTOR 0.75

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
    size_t num_entries;
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

static int hash_rehash(HashTable *table) {
    if (!table) return -1;
    size_t new_buckets_count = table->num_buckets * 2;
    if (new_buckets_count < table->num_buckets) return -1; // Overflow protection

    HashEntry **new_buckets = (HashEntry**)calloc(new_buckets_count, sizeof(HashEntry*));
    if (!new_buckets) return -1;

    for (size_t i = 0; i < table->num_buckets; i++) {
        HashEntry *entry = table->buckets[i];
        while (entry) {
            HashEntry *next = entry->next;
            size_t new_idx = hash_func(entry->key, new_buckets_count);
            entry->next = new_buckets[new_idx];
            new_buckets[new_idx] = entry;
            entry = next;
        }
    }

    free(table->buckets);
    table->buckets = new_buckets;
    table->num_buckets = new_buckets_count;
    return 0;
}

HashTable* hash_create_with_capacity(size_t initial_buckets) {
    HashTable *table = (HashTable*)malloc(sizeof(HashTable));
    if (!table) return NULL;

    table->num_buckets = (initial_buckets > 0) ? initial_buckets : DEFAULT_INITIAL_BUCKETS;
    table->num_entries = 0;
    table->buckets = (HashEntry**)calloc(table->num_buckets, sizeof(HashEntry*));
    if (!table->buckets) {
        free(table);
        return NULL;
    }
    return table;
}

HashTable* hash_create(void) {
    return hash_create_with_capacity(DEFAULT_INITIAL_BUCKETS);
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
            // Key found. Find position in sorted postings by document_id
            size_t low = 0;
            size_t high = entry->postings_count;
            int found = 0;
            while (low < high) {
                size_t mid = low + (high - low) / 2;
                if (entry->postings[mid].document_id == posting.document_id) {
                    entry->postings[mid].frequency += posting.frequency;
                    found = 1;
                    break;
                } else if (entry->postings[mid].document_id < posting.document_id) {
                    low = mid + 1;
                } else {
                    high = mid;
                }
            }

            if (found) return 0;

            // Not found, insert at index 'low' to maintain sorted order
            if (entry->postings_count >= entry->postings_capacity) {
                size_t new_cap = entry->postings_capacity == 0 ? 4 : entry->postings_capacity * 2;
                Posting *new_postings = (Posting*)realloc(entry->postings, new_cap * sizeof(Posting));
                if (!new_postings) return -1;
                entry->postings = new_postings;
                entry->postings_capacity = new_cap;
            }

            if (low < entry->postings_count) {
                memmove(&entry->postings[low + 1], &entry->postings[low],
                        (entry->postings_count - low) * sizeof(Posting));
            }
            entry->postings[low] = posting;
            entry->postings_count++;
            return 0;
        }
        entry = entry->next;
    }

    // Key not found in table. Check load factor for resizing
    if ((double)(table->num_entries + 1) / (double)table->num_buckets > MAX_LOAD_FACTOR) {
        if (hash_rehash(table) == 0) {
            idx = hash_func(key, table->num_buckets);
        }
    }

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
    table->num_entries++;

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

void hash_for_each(const HashTable *table, HashEntryVisitor visitor, void *user_data) {
    if (!table || !visitor) return;
    for (size_t i = 0; i < table->num_buckets; i++) {
        HashEntry *entry = table->buckets[i];
        while (entry) {
            visitor(entry->key, entry->postings, entry->postings_count, user_data);
            entry = entry->next;
        }
    }
}

size_t hash_size(const HashTable *table) {
    return table ? table->num_entries : 0;
}

size_t hash_num_buckets(const HashTable *table) {
    return table ? table->num_buckets : 0;
}

double hash_load_factor(const HashTable *table) {
    if (!table || table->num_buckets == 0) return 0.0;
    return (double)table->num_entries / (double)table->num_buckets;
}

double hash_insert_timed(HashTable *table, const char *key, Posting posting) {
    LARGE_INTEGER start, end, freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);
    hash_insert(table, key, posting);
    QueryPerformanceCounter(&end);
    return (double)(end.QuadPart - start.QuadPart) / (double)freq.QuadPart;
}

double hash_lookup_timed(const HashTable *table, const char *key, Posting **out_postings, size_t *out_count) {
    LARGE_INTEGER start, end, freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);
    *out_postings = hash_lookup(table, key);
    *out_count = hash_lookup_count(table, key);
    QueryPerformanceCounter(&end);
    return (double)(end.QuadPart - start.QuadPart) / (double)freq.QuadPart;
}
