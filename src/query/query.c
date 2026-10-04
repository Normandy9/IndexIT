#include <stdio.h>
#include <stddef.h>
#include "query.h"
#include "inverted_index.h"

/*
 * Query engine for the current project stage.
 *
 * The index is supplied by the indexing layer through query_set_index().
 * run_query() performs exact-term lookup using the inverted index.
 */

static const InvertedIndex *g_index = NULL;

void query_set_index(const InvertedIndex *index)
{
    g_index = index;
}

void query_init(void)
{
    g_index = NULL;
}

void query_cleanup(void)
{
    g_index = NULL;
}

int run_query(const char *query)
{
    if (query == NULL || query[0] == '\0')
    {
        fprintf(stderr, "Error: search query cannot be empty.\n");
        return 1;
    }

    if (g_index == NULL)
    {
        printf("No search index is loaded.\n");
        return 2;
    }

    Posting *posting = index_lookup(g_index, query);

    if (posting == NULL)
    {
        printf("No results found for: %s\n", query);
        return 0;
    }

    /*
     * The current Posting API represents one posting at a time.
     * The index implementation can later expose the full posting list.
     * For now, report the first matching posting returned by the API.
     */
    printf("Results for \"%s\":\n", query);
    printf("  Document ID: %llu, frequency: %u\n",
           (unsigned long long)posting->document_id,
           posting->frequency);

    return 0;
}
