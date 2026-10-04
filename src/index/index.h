#ifndef INDEXIT_INDEX_IMPL_H
#define INDEXIT_INDEX_IMPL_H

#include "inverted_index.h"

/* Create a small in-memory index for the current integration stage. */
InvertedIndex *index_create(void);
void index_destroy(InvertedIndex *index);
int index_insert(InvertedIndex *index, const char *term, Posting posting);
Posting *index_lookup(const InvertedIndex *index, const char *term);

#endif
