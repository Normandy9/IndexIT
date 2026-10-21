#include "trie.h"
#include <stdlib.h>
#include <string.h>

#define ALPHABET_SIZE 36 // a-z (26) + 0-9 (10)

typedef struct TrieNode {
    struct TrieNode *children[ALPHABET_SIZE];
    Posting *postings;
    size_t postings_count;
    size_t postings_capacity;
} TrieNode;

struct Trie {
    TrieNode *root;
};

static int char_to_index(char c) {
    if (c >= 'a' && c <= 'z') return c - 'a';
    if (c >= '0' && c <= '9') return c - '0' + 26;
    return -1;
}

static TrieNode* create_node(void) {
    TrieNode *node = (TrieNode*)malloc(sizeof(TrieNode));
    if (node) {
        memset(node->children, 0, sizeof(node->children));
        node->postings = NULL;
        node->postings_count = 0;
        node->postings_capacity = 0;
    }
    return node;
}

static void destroy_node(TrieNode *node) {
    if (node) {
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            if (node->children[i]) {
                destroy_node(node->children[i]);
            }
        }
        if (node->postings) free(node->postings);
        free(node);
    }
}

Trie* trie_create(void) {
    Trie *trie = (Trie*)malloc(sizeof(Trie));
    if (trie) {
        trie->root = create_node();
    }
    return trie;
}

void trie_destroy(Trie *trie) {
    if (trie) {
        destroy_node(trie->root);
        free(trie);
    }
}

int trie_insert(Trie *trie, const char *term, Posting posting) {
    if (!trie || !trie->root || !term) return -1;
    
    TrieNode *current = trie->root;
    while (*term) {
        int index = char_to_index(*term);
        if (index == -1) {
            term++;
            continue; // Skip invalid characters (though tokenizer should filter them)
        }
        if (!current->children[index]) {
            current->children[index] = create_node();
            if (!current->children[index]) return -1; // Out of memory
        }
        current = current->children[index];
        term++;
    }
    
    // Add posting
    // Check if posting already exists for this document
    for (size_t i = 0; i < current->postings_count; i++) {
        if (current->postings[i].document_id == posting.document_id) {
            current->postings[i].frequency += posting.frequency;
            return 0;
        }
    }
    
    if (current->postings_count >= current->postings_capacity) {
        size_t new_cap = current->postings_capacity == 0 ? 4 : current->postings_capacity * 2;
        Posting *new_postings = (Posting*)realloc(current->postings, new_cap * sizeof(Posting));
        if (!new_postings) return -1;
        current->postings = new_postings;
        current->postings_capacity = new_cap;
    }
    
    current->postings[current->postings_count++] = posting;
    return 0;
}

// Note: The interface returns a pointer. Since it's a single return, we return the internal pointer.
// The caller must not free it. But we don't have a way to return the count through this interface!
// We will need to return the first posting for now, but a real index needs to return the array and count.
Posting* trie_search(const Trie *trie, const char *term) {
    if (!trie || !trie->root || !term) return NULL;
    
    TrieNode *current = trie->root;
    while (*term) {
        int index = char_to_index(*term);
        if (index == -1) {
            term++;
            continue;
        }
        if (!current->children[index]) {
            return NULL;
        }
        current = current->children[index];
        term++;
    }
    
    return current->postings;
}

// Extension to the interface to get counts
size_t trie_search_count(const Trie *trie, const char *term) {
    if (!trie || !trie->root || !term) return 0;
    
    TrieNode *current = trie->root;
    while (*term) {
        int index = char_to_index(*term);
        if (index == -1) {
            term++;
            continue;
        }
        if (!current->children[index]) {
            return 0;
        }
        current = current->children[index];
        term++;
    }
    
    return current->postings_count;
}

// Prefix search support
static void collect_postings(TrieNode *node, Posting **results, size_t *count, size_t *capacity) {
    if (!node) return;
    for (size_t i = 0; i < node->postings_count; i++) {
        int found = 0;
        for (size_t j = 0; j < *count; j++) {
            if ((*results)[j].document_id == node->postings[i].document_id) {
                (*results)[j].frequency += node->postings[i].frequency;
                found = 1;
                break;
            }
        }
        if (!found) {
            if (*count >= *capacity) {
                *capacity = (*capacity == 0) ? 16 : (*capacity * 2);
                *results = (Posting*)realloc(*results, *capacity * sizeof(Posting));
            }
            (*results)[(*count)++] = node->postings[i];
        }
    }
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i]) {
            collect_postings(node->children[i], results, count, capacity);
        }
    }
}

Posting* trie_prefix_search(const Trie *trie, const char *prefix, size_t *out_count) {
    if (!trie || !trie->root || !prefix || !out_count) return NULL;
    *out_count = 0;
    
    TrieNode *current = trie->root;
    while (*prefix) {
        int index = char_to_index(*prefix);
        if (index == -1) { prefix++; continue; }
        if (!current->children[index]) return NULL;
        current = current->children[index];
        prefix++;
    }
    
    Posting *results = NULL;
    size_t count = 0, capacity = 0;
    collect_postings(current, &results, &count, &capacity);
    *out_count = count;
    return results;
}

// Benchmarking wrappers
#include <windows.h>
double trie_insert_timed(Trie *trie, const char *term, Posting posting) {
    LARGE_INTEGER start, end, freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);
    trie_insert(trie, term, posting);
    QueryPerformanceCounter(&end);
    return (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
}

double trie_search_timed(const Trie *trie, const char *term, Posting **out_postings, size_t *out_count) {
    LARGE_INTEGER start, end, freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);
    *out_postings = trie_search(trie, term);
    *out_count = trie_search_count(trie, term);
    QueryPerformanceCounter(&end);
    return (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
}
