#include <assert.h>
#include <stdio.h>

/*
 * Lightweight C test runner.
 *
 * We use the standard assert() library for the initial project
 * foundation, so no external testing framework is required yet.
 */

static void test_basic_assertion(void)
{
    int value = 2 + 3;

    assert(value == 5);
}

static void test_project_test_runner(void)
{
    int project_ready = 1;

    assert(project_ready == 1);
}

int main(void)
{
    test_basic_assertion();
    test_project_test_runner();

    printf("All IndexIt tests passed.\n");

    return 0;
}
