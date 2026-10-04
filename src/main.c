#include <stdio.h>
#include <string.h>

#include "benchmark.h"
#include "query.h"
#include "inverted_index.h"
#include "dataset.h"

static InvertedIndex *g_index = NULL;

static void usage(const char *program)
{
    printf("IndexIt - Optimized Local Document Search Engine\n\n");
    printf("Usage:\n");
    printf("  %s index <folder>\n", program);
    printf("  %s search <folder> <query>\n", program);
    printf("  %s benchmark <folder> <query>\n", program);
    printf("  %s stats\n", program);
    printf("  %s clean\n", program);
}

static int command_index(const char *folder)
{
    if (folder == NULL || folder[0] == '\0')
    {
        fprintf(stderr, "Error: folder path is required.\n");
        return 1;
    }

    if (g_index != NULL)
    {
        index_destroy(g_index);
        g_index = NULL;
    }

    g_index = index_create();

    if (g_index == NULL)
    {
        fprintf(stderr, "Error: could not create index.\n");
        return 1;
    }

    if (index_dataset_folder(folder, g_index) != 0)
    {
        fprintf(stderr, "Error: could not index dataset.\n");
        return 1;
    }

    query_set_index(g_index);

    printf("Dataset indexed successfully: %s\n", folder);

    return 0;
}

static int command_benchmark(const char *folder, const char *query)
{
    const int runs = 100;
    double total = 0.0;
    double min = 999999999.0;
    double max = 0.0;

    if (folder == NULL || folder[0] == '\0')
    {
        fprintf(stderr, "Error: folder path is required.\n");
        return 1;
    }

    if (query == NULL || query[0] == '\0')
    {
        fprintf(stderr, "Error: search query is required.\n");
        return 1;
    }

    if (g_index != NULL)
    {
        index_destroy(g_index);
        g_index = NULL;
    }

    g_index = index_create();

    if (g_index == NULL)
    {
        fprintf(stderr, "Error: could not create index.\n");
        return 1;
    }

    printf("Indexing dataset...\n");

    if (index_dataset_folder(folder, g_index) != 0)
    {
        fprintf(stderr, "Error: indexing failed.\n");
        return 1;
    }

    query_set_index(g_index);

    printf("Running %d benchmark queries...\n", runs);

    for (int i = 0; i < runs; i++)
    {
        double elapsed;

        timer_start();

        run_query(query);

        elapsed = timer_stop_ms();

        total += elapsed;

        if (elapsed < min)
            min = elapsed;

        if (elapsed > max)
            max = elapsed;
    }

    printf("\n===== IndexIT Benchmark =====\n");
    printf("Dataset : %s\n", folder);
    printf("Query   : %s\n", query);
    printf("Runs    : %d\n", runs);
    printf("Average : %.3f ms\n", total / runs);
    printf("Minimum : %.3f ms\n", min);
    printf("Maximum : %.3f ms\n", max);

    return 0;
}

static int command_search(const char *folder, const char *query)
{
    double elapsed;
    int result;

    if (folder == NULL || folder[0] == '\0')
    {
        fprintf(stderr, "Error: folder path is required.\n");
        return 1;
    }

    if (query == NULL || query[0] == '\0')
    {
        fprintf(stderr, "Error: search query is required.\n");
        return 1;
    }

    if (g_index != NULL)
    {
        index_destroy(g_index);
        g_index = NULL;
    }

    g_index = index_create();

    if (g_index == NULL)
    {
        fprintf(stderr, "Error: could not create index.\n");
        return 1;
    }

    if (index_dataset_folder(folder, g_index) != 0)
    {
        fprintf(stderr, "Error: could not index dataset.\n");
        return 1;
    }

    query_set_index(g_index);

    timer_start();

    result = run_query(query);

    elapsed = timer_stop_ms();

    printf("Query time: %.3f ms\n", elapsed);

    return result;
}

static int command_stats(void)
{
    printf("Statistics module is not implemented in this branch yet.\n");
    return 0;
}

static int command_clean(void)
{
    printf("Clean module is not implemented in this branch yet.\n");
    return 0;
}

int main(int argc, char **argv)
{
    int result = 0;

    if (argc < 2)
    {
        usage(argv[0]);
        return 1;
    }

    benchmark_init();
    query_init();

    if (strcmp(argv[1], "--help") == 0 ||
        strcmp(argv[1], "-h") == 0)
    {
        usage(argv[0]);
    }
    else if (strcmp(argv[1], "index") == 0)
    {
        if (argc != 3)
        {
            fprintf(stderr, "Usage: %s index <folder>\n", argv[0]);
            result = 1;
        }
        else
        {
            result = command_index(argv[2]);
        }
    }
    else if (strcmp(argv[1], "search") == 0)
    {
        if (argc < 4)
        {
            fprintf(stderr,
                    "Usage: %s search <folder> <query>\n",
                    argv[0]);
            result = 1;
        }
        else
        {
            char query[1024];
            size_t used = 0;

            query[0] = '\0';

            for (int i = 3; i < argc; ++i)
            {
                size_t len = strlen(argv[i]);

                if (used + len + (i > 3 ? 1 : 0) >= sizeof(query))
                {
                    fprintf(stderr, "Error: query is too long.\n");
                    result = 1;
                    break;
                }

                if (i > 3)
                    query[used++] = ' ';

                memcpy(query + used, argv[i], len);
                used += len;
                query[used] = '\0';
            }

            if (result == 0)
            {
                result = command_search(argv[2], query);
            }
        }
    }
    else if (strcmp(argv[1], "benchmark") == 0)
    {
        if (argc < 4)
        {
            fprintf(stderr,
                    "Usage: %s benchmark <folder> <query>\n",
                    argv[0]);
            result = 1;
        }
        else
        {
            char query[1024];
            size_t used = 0;

            query[0] = '\0';

            for (int i = 3; i < argc; ++i)
            {
                size_t len = strlen(argv[i]);

                if (used + len + (i > 3 ? 1 : 0) >= sizeof(query))
                {
                    fprintf(stderr, "Error: query is too long.\n");
                    result = 1;
                    break;
                }

                if (i > 3)
                    query[used++] = ' ';

                memcpy(query + used, argv[i], len);
                used += len;
                query[used] = '\0';
            }

            if (result == 0)
            {
                result = command_benchmark(argv[2], query);
            }
        }
    }
    else if (strcmp(argv[1], "stats") == 0)
    {
        if (argc != 2)
        {
            fprintf(stderr, "Usage: %s stats\n", argv[0]);
            result = 1;
        }
        else
        {
            result = command_stats();
        }
    }
    else if (strcmp(argv[1], "clean") == 0)
    {
        if (argc != 2)
        {
            fprintf(stderr, "Usage: %s clean\n", argv[0]);
            result = 1;
        }
        else
        {
            result = command_clean();
        }
    }
    else
    {
        fprintf(stderr, "Unknown command: %s\n\n", argv[1]);
        usage(argv[0]);
        result = 1;
    }

    query_cleanup();
    benchmark_cleanup();

    if (g_index != NULL)
    {
        index_destroy(g_index);
        g_index = NULL;
    }

    return result;
}