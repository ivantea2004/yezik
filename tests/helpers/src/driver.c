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
    getcwd(wd, sizeof(wd));

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

static int run_test_by_index(size_t index)
{
    (void)index;
    return 0;
}

static int run_all_tests()
{
    return 1;
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
            return run_test_by_index(index);
        }
    }
    else if (argc == 2 && strcmp(argv[1], "run") == 0)
    {
        return run_all_tests();
    }

    fprintf(stderr, "Test driver was called with invalid arguments.\n");
    for (int i = 0; i < argc; i++)
    {
        fprintf(stderr, "argv[%d] = \"%s\"\n", i, argv[i]);
    }
    fprintf(stderr, "Valid arguments are:\n");
    fprintf(stderr, "<driver> generate-ctest <ctest-include-file> <working_directory> - to generate ctest include file\n");
    fprintf(stderr, "<driver> run <index> - to run test by index\n");
    fprintf(stderr, "<driver> run - to run all tests\n");

    return 1;
}
