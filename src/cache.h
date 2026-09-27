#ifndef INDEXIT_CACHE_H
#define INDEXIT_CACHE_H

/*
 * Cache interface for IndexIt.
 *
 * Current stage:
 * - CacheEntry is defined.
 * - cache_get() and cache_put() interfaces are declared.
 *
 * The actual cache implementation (LRU, storage, cleanup, etc.)
 * will be added later.
 */

typedef struct
{
    char *key;
    char *value;
} CacheEntry;

/*
 * Get a cache entry using its key.
 *
 * Returns:
 *   Pointer to the matching CacheEntry if found.
 *   NULL if the key is not present.
 */
CacheEntry *cache_get(const char *key);

/*
 * Add or update a cache entry.
 */
void cache_put(const char *key, const char *value);

#endif /* INDEXIT_CACHE_H */
