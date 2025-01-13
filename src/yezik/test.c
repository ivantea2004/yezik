#include <yezik/test.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <yezik/yezik.h>

#if YEZIK_BUILD_TESTING
#include <yezik/yezik_test_suite.h>

static int test_entry_point_(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    size_t cases_count = sizeof(yezik_test_cases) / sizeof(yezik_test_cases[0]) - 1;
    for (size_t i = 0; i < cases_count; i++)
    {
        yezik_test_cases[i]();
    }
    return 0;
}

static int run_test_(size_t index)
{
    size_t cases_count = sizeof(yezik_test_cases) / sizeof(yezik_test_cases[0]) - 1;
    if (index >= cases_count)
    {
        fprintf(stderr, "%" PRIuPTR " is out of range.\n", index);
        return 1;
    }
    return yezik_test_cases[index]();
}

static int generate_ctest_include_file_(const char *include_path, const char *work_dir)
{
    YEZIK_FN_BEGIN();
    FILE* include = NULL;

    YEZIK_TRY_ERRNO((include = fopen(include_path, "w")), include_path);

    YEZIK_FN_CLEANUP();
    if (include) fclose(include);
    YEZIK_FN_END();
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Invalid arguments.\n");
        return 1;
    }
    char *cmd = argv[1];
    if (strcmp(cmd, "gen") == 0)
    {
    }
    else if (strcmp(cmd, "run") == 0)
    {
        if (argc == 3)
        {
            size_t index;
            if (sscanf(argv[2], "%" SCNuPTR, &index) != 1)
            {
                fprintf(stderr, "%s is not a valid test index.\n", argv[2]);
                return 1;
            }
            return run_test_(index);
        }
        else
        {
            fprintf(stderr, "Invalid number of arguments.\n");
            return 1;
        }
    }
    else
    {
        fprintf(stderr, "Invalid command %s.\n", cmd);
        return 1;
    }
}
#endif
