#include "trie.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>

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
    c = (char)tolower((unsigned char)c);
    if (c >= 'a' && c <= 'z') return c - 'a';
    if (c >= '0' && c <= '9') return c - '0' + 26;
    return -1;
}

static char index_to_char(int idx) {
    if (idx >= 0 && idx < 26) return (char)('a' + idx);
    if (idx >= 26 && idx < 36) return (char)('0' + (idx - 26));
    return '?';
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
            continue;
        }
        if (!current->children[index]) {
            current->children[index] = create_node();
            if (!current->children[index]) return -1;
        }
        current = current->children[index];
        term++;
    }
    
    // Add posting in sorted order by document_id
    size_t low = 0;
    size_t high = current->postings_count;
    int found = 0;
    while (low < high) {
        size_t mid = low + (high - low) / 2;
        if (current->postings[mid].document_id == posting.document_id) {
            current->postings[mid].frequency += posting.frequency;
            found = 1;
            break;
        } else if (current->postings[mid].document_id < posting.document_id) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }

    if (found) return 0;

    if (current->postings_count >= current->postings_capacity) {
        size_t new_cap = current->postings_capacity == 0 ? 4 : current->postings_capacity * 2;
        Posting *new_postings = (Posting*)realloc(current->postings, new_cap * sizeof(Posting));
        if (!new_postings) return -1;
        current->postings = new_postings;
        current->postings_capacity = new_cap;
    }

    if (low < current->postings_count) {
        memmove(&current->postings[low + 1], &current->postings[low],
                (current->postings_count - low) * sizeof(Posting));
    }
    current->postings[low] = posting;
    current->postings_count++;
    return 0;
}

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

// Prefix search support (sorted insertion)
static void collect_postings(TrieNode *node, Posting **results, size_t *count, size_t *capacity) {
    if (!node) return;
    for (size_t i = 0; i < node->postings_count; i++) {
        size_t low = 0;
        size_t high = *count;
        int found = 0;
        while (low < high) {
            size_t mid = low + (high - low) / 2;
            if ((*results)[mid].document_id == node->postings[i].document_id) {
                (*results)[mid].frequency += node->postings[i].frequency;
                found = 1;
                break;
            } else if ((*results)[mid].document_id < node->postings[i].document_id) {
                low = mid + 1;
            } else {
                high = mid;
            }
        }
        if (!found) {
            if (*count >= *capacity) {
                *capacity = (*capacity == 0) ? 16 : (*capacity * 2);
                *results = (Posting*)realloc(*results, *capacity * sizeof(Posting));
            }
            if (low < *count) {
                memmove(&(*results)[low + 1], &(*results)[low], (*count - low) * sizeof(Posting));
            }
            (*results)[low] = node->postings[i];
            (*count)++;
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

// Autocomplete suggestions ranked by document frequency
typedef struct {
    char term[128];
    size_t doc_frequency;
} AutocompleteCandidate;

static void collect_terms(TrieNode *node, char *buffer, int depth, AutocompleteCandidate **candidates, size_t *count, size_t *cap) {
    if (!node) return;
    if (node->postings_count > 0 && depth > 0) {
        buffer[depth] = '\0';
        if (*count >= *cap) {
            *cap = (*cap == 0) ? 32 : (*cap * 2);
            *candidates = (AutocompleteCandidate*)realloc(*candidates, *cap * sizeof(AutocompleteCandidate));
        }
        strncpy((*candidates)[*count].term, buffer, sizeof((*candidates)[*count].term) - 1);
        (*candidates)[*count].term[sizeof((*candidates)[*count].term) - 1] = '\0';
        (*candidates)[*count].doc_frequency = node->postings_count;
        (*count)++;
    }
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i] && depth < 120) {
            buffer[depth] = index_to_char(i);
            collect_terms(node->children[i], buffer, depth + 1, candidates, count, cap);
        }
    }
}

static int compare_candidates(const void *a, const void *b) {
    const AutocompleteCandidate *ca = (const AutocompleteCandidate*)a;
    const AutocompleteCandidate *cb = (const AutocompleteCandidate*)b;
    if (cb->doc_frequency != ca->doc_frequency) {
        return (cb->doc_frequency > ca->doc_frequency) ? 1 : -1;
    }
    return strcmp(ca->term, cb->term);
}

size_t trie_autocomplete(const Trie *trie, const char *prefix, TermSuggestion *out_suggestions, size_t max_suggestions) {
    if (!trie || !trie->root || !prefix || !out_suggestions || max_suggestions == 0) return 0;

    TrieNode *current = trie->root;
    char buffer[128];
    int depth = 0;

    const char *p = prefix;
    while (*p) {
        int index = char_to_index(*p);
        if (index == -1) { p++; continue; }
        if (!current->children[index]) return 0;
        buffer[depth++] = index_to_char(index);
        current = current->children[index];
        p++;
    }
    buffer[depth] = '\0';

    AutocompleteCandidate *candidates = NULL;
    size_t count = 0, cap = 0;
    collect_terms(current, buffer, depth, &candidates, &count, &cap);

    if (count == 0) {
        if (candidates) free(candidates);
        return 0;
    }

    qsort(candidates, count, sizeof(AutocompleteCandidate), compare_candidates);

    size_t result_count = (count < max_suggestions) ? count : max_suggestions;
    for (size_t i = 0; i < result_count; i++) {
        strncpy(out_suggestions[i].term, candidates[i].term, sizeof(out_suggestions[i].term) - 1);
        out_suggestions[i].term[sizeof(out_suggestions[i].term) - 1] = '\0';
        out_suggestions[i].doc_frequency = candidates[i].doc_frequency;
    }

    free(candidates);
    return result_count;
}

double trie_insert_timed(Trie *trie, const char *term, Posting posting) {
    LARGE_INTEGER start, end, freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);
    trie_insert(trie, term, posting);
    QueryPerformanceCounter(&end);
    return (double)(end.QuadPart - start.QuadPart) / (double)freq.QuadPart;
}

double trie_search_timed(const Trie *trie, const char *term, Posting **out_postings, size_t *out_count) {
    LARGE_INTEGER start, end, freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);
    *out_postings = trie_search(trie, term);
    *out_count = trie_search_count(trie, term);
    QueryPerformanceCounter(&end);
    return (double)(end.QuadPart - start.QuadPart) / (double)freq.QuadPart;
}
