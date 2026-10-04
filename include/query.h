#ifndef QUERY_H
#define QUERY_H

#include "inverted_index.h"

void query_init(void);
void query_cleanup(void);
void query_set_index(const InvertedIndex *index);
int run_query(const char *query);

#endif
