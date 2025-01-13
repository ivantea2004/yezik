/*
    Test driver for testing system

    Should be added as a source to test executable

 */
#include <tests/test_config.h>
#include <helpers/test_suite.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#define SUBPROCESS_IMPL
#include <subprocess/subprocess.h>

#include <unistd.h>

static int generate_ctest(const char *self, const char *ctest_path, const char *work_dir)
{

    FILE *ctest = fopen(ctest_path, "w");
    if (!ctest)
    {
        perror(ctest_path);
        return 1;
    }

    size_t tests_count = sizeof(test_suite) / sizeof(test_suite[0]) - 1;

    printf("ctest include = %s\n", ctest_path);

    char wd[1000];
    (void)getcwd(wd, sizeof(wd));

    for (size_t i = 0; i < tests_count; i++)
    {
        TestInfo info = test_suite[i]();
        printf("Test \"%s\"\n", info.test_name);
        fprintf(ctest, "add_test(%s %s/%s run %" PRIuPTR ")\n", info.test_name, wd, self, i);
        fprintf(ctest, "set_tests_properties(%s PROPERTIES WORKING_DIRECTORY %s)\n", info.test_name, work_dir);
    }

    fclose(ctest);

    return 0;
}

void test_failed()
{
    exit(1);
}

static int run_test_by_index_impl(size_t index)
{
    TestInfo info = test_suite[index]();
    info.test();

    // if here it means that test did not crashed, aborted, exited or used test_failed()
    // therefore it can be considered a test pass
    return 0;
}

static int run_test_by_index(const char *self, size_t index)
{
    size_t tests_count = sizeof(test_suite) / sizeof(test_suite[0]) - 1;

    if (index >= tests_count)
    {
        fprintf(stderr, "Invalid test index.\n");
        fprintf(stderr, "Valid indicies are [0, %" PRIuPTR ").\n", tests_count);
        return 1;
    }

    TestInfo info = test_suite[index]();

    if (info.should_fail)
    {
        char num[100];
        snprintf(num, sizeof(num), "%" PRIuPTR, index);
        const char *args[4] = {self, "run", num, "impl"};
        int exit_code = 0;
        if (subprocess_run(4, args, &exit_code) != 0 || exit_code != 0)
        {
            // test failed means ok
            return 0;
        }
        printf("Test did not fail but it is expected to fail.\n");
        printf("Restarting the test to be able to debug it.\n");
        return run_test_by_index_impl(index);
    }
    else
    {
        return run_test_by_index_impl(index);
    }
}

static int run_all_tests(const char *self)
{
    int results[sizeof(test_suite) / sizeof(test_suite[0]) - 1] = {0};
    size_t tests_count = sizeof(test_suite) / sizeof(test_suite[0]) - 1;
    for (size_t i = 0; i < tests_count; i++)
    {
        TestInfo info = test_suite[i]();
        char num[100];
        snprintf(num, sizeof(num), "%" PRIuPTR, i);
        const char *args[3] = {self, "run", num};
        int exit_code;
        printf("Starting test \"%s\"\n", info.test_name);
        int res = subprocess_run(3, args, &exit_code);

        if (res < 0)
        {
            results[i] = -1;
            printf("Failed to run subprocess for test.\n");
        }
        else if (res > 0 || exit_code != 0)
        {
            results[i] = 1;
            printf("Failed.\n");
        }
        else
        {
            results[i] = 0;
            printf("Passed.\n");
        }
    }
    int passed = 0;
    int failed = 0;
    int errors = 0;
    for (size_t i = 0; i < sizeof(test_suite) / sizeof(test_suite[0]) - 1; i++)
    {
        if (results[i] == 0)
            passed++;
        if (results[i] > 0)
            failed++;
        if (results[i] < 0)
            errors++;
    }
    printf("Total: %d passed, %d failed, %d could not run.\n", passed, failed, errors);
    return 0;
}

int main(int argc, char **argv)
{

    if (argc == 4 && strcmp(argv[1], "generate-ctest") == 0)
    {
        char *ctest_include = argv[2];
        char *work_dir = argv[3];
        return generate_ctest(argv[0], ctest_include, work_dir);
    }
    else if (argc == 3 && strcmp(argv[1], "run") == 0)
    {
        size_t index = 0;
        if (sscanf(argv[2], "%" SCNuPTR, &index) == 1)
        {
            return run_test_by_index(argv[0], index);
        }
    }
    else if (argc == 4 && strcmp(argv[1], "run") && strcmp(argv[3], "impl") == 0)
    {
        size_t index = 0;
        if (sscanf(argv[2], "%" SCNuPTR, &index) == 1)
        {
            return run_test_by_index_impl(index);
        }
    }
    else if (argc == 2 && strcmp(argv[1], "run") == 0)
    {
        return run_all_tests(argv[0]);
    }

    fprintf(stderr, "Test driver was called with invalid arguments.\n");
    for (int i = 0; i < argc; i++)
    {
        fprintf(stderr, "argv[%d] = \"%s\"\n", i, argv[i]);
    }
    fprintf(stderr, "Valid arguments are:\n");
    fprintf(stderr, "<driver> generate-ctest <ctest-include-file> <working_directory> - to generate ctest include file\n");
    fprintf(stderr, "<driver> run <index> - to run test by index\n");
    fprintf(stderr, "<driver> run <index> impl - to run test by index implementation\n");
    fprintf(stderr, "<driver> run - to run all tests\n");

    return 1;
}
