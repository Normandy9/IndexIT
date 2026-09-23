#ifndef INVERTED_INDEX_H
#define INVERTED_INDEX_H

#include "token.h"

typedef struct InvertedIndex InvertedIndex;

InvertedIndex *index_create(void);
void index_destroy(InvertedIndex *index);

int index_insert(InvertedIndex *index, const char *term, Posting posting);
Posting *index_lookup(const InvertedIndex *index, const char *term);

#endif