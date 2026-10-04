#include "benchmark.h"

#ifdef _WIN32

#include <windows.h>

static LARGE_INTEGER g_start;
static LARGE_INTEGER g_frequency;

void benchmark_init(void)
{
    QueryPerformanceFrequency(&g_frequency);
}

void benchmark_cleanup(void)
{
    /* No resources to release. */
}

void timer_start(void)
{
    if (g_frequency.QuadPart == 0)
        QueryPerformanceFrequency(&g_frequency);

    QueryPerformanceCounter(&g_start);
}

double timer_stop_ms(void)
{
    LARGE_INTEGER end;

    if (g_frequency.QuadPart == 0)
        QueryPerformanceFrequency(&g_frequency);

    QueryPerformanceCounter(&end);

    return ((double)(end.QuadPart - g_start.QuadPart) /
            (double)g_frequency.QuadPart) * 1000.0;
}

#else

#define _POSIX_C_SOURCE 200809L
#include <time.h>

static struct timespec g_start;

void benchmark_init(void) {}
void benchmark_cleanup(void) {}

void timer_start(void)
{
    clock_gettime(CLOCK_MONOTONIC, &g_start);
}

double timer_stop_ms(void)
{
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);

    long seconds = end.tv_sec - g_start.tv_sec;
    long nanoseconds = end.tv_nsec - g_start.tv_nsec;

    return (double)seconds * 1000.0 +
           (double)nanoseconds / 1000000.0;
}

#endif
