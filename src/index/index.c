/*
 * Minimal in-memory inverted-index implementation.
 *
 * This is an integration implementation for the current foundation stage.
 * It keeps the public API from include/inverted_index.h usable by query.c.
 * The full production index can replace this module later.
 */

#include <stdlib.h>
#include <string.h>
#include "inverted_index.h"

typedef struct IndexNode
{
    char *term;
    Posting posting;
    struct IndexNode *next;
} IndexNode;

struct InvertedIndex
{
    IndexNode *head;
};

static char *duplicate_string(const char *s)
{
    size_t len;
    char *copy;

    if (s == NULL)
        return NULL;

    len = strlen(s) + 1;
    copy = (char *)malloc(len);

    if (copy != NULL)
        memcpy(copy, s, len);

    return copy;
}

InvertedIndex *index_create(void)
{
    return (InvertedIndex *)calloc(1, sizeof(InvertedIndex));
}

void index_destroy(InvertedIndex *index)
{
    IndexNode *node;
    IndexNode *next;

    if (index == NULL)
        return;

    node = index->head;

    while (node != NULL)
    {
        next = node->next;
        free(node->term);
        free(node);
        node = next;
    }

    free(index);
}

int index_insert(InvertedIndex *index, const char *term, Posting posting)
{
    IndexNode *node;

    if (index == NULL || term == NULL || term[0] == '\0')
        return 1;

    for (node = index->head; node != NULL; node = node->next)
    {
        if (strcmp(node->term, term) == 0)
        {
            node->posting = posting;
            return 0;
        }
    }

    node = (IndexNode *)malloc(sizeof(IndexNode));

    if (node == NULL)
        return 2;

    node->term = duplicate_string(term);

    if (node->term == NULL)
    {
        free(node);
        return 2;
    }

    node->posting = posting;
    node->next = index->head;
    index->head = node;

    return 0;
}

Posting *index_lookup(const InvertedIndex *index, const char *term)
{
    IndexNode *node;

    if (index == NULL || term == NULL)
        return NULL;

    for (node = index->head; node != NULL; node = node->next)
    {
        if (strcmp(node->term, term) == 0)
            return &node->posting;
    }

    return NULL;
}
