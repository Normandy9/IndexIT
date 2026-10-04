#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "inverted_index.h"
#include "query.h"
#include "benchmark.h"

static void test_index_lookup(void)
{
    InvertedIndex *index = index_create();
    Posting posting = {42, 7};
    Posting *found;

    assert(index != NULL);
    assert(index_insert(index, "machine", posting) == 0);

    found = index_lookup(index, "machine");

    assert(found != NULL);
    assert(found->document_id == 42);
    assert(found->frequency == 7);

    assert(index_lookup(index, "missing") == NULL);

    index_destroy(index);
}

static void test_query_no_index(void)
{
    query_init();

    assert(run_query("missing") == 2);

    query_cleanup();
}

static void test_query_with_index(void)
{
    InvertedIndex *index = index_create();
    Posting posting = {100, 3};

    assert(index != NULL);
    assert(index_insert(index, "algorithm", posting) == 0);

    query_init();
    query_set_index(index);

    assert(run_query("algorithm") == 0);
    assert(run_query("does-not-exist") == 0);

    query_cleanup();
    index_destroy(index);
}

static void test_timer(void)
{
    double elapsed;

    benchmark_init();

    timer_start();

    for (volatile int i = 0; i < 100000; ++i)
        ;

    elapsed = timer_stop_ms();

    assert(elapsed >= 0.0);

    benchmark_cleanup();
}

int main(void)
{
    test_index_lookup();
    test_query_no_index();
    test_query_with_index();
    test_timer();

    printf("All IndexIt tests passed.\n");
    return 0;
}
