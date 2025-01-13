
#define SUBPROCESS_IMPL
#include <subprocess/subprocess.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int entry(int argc, char **argv)
{
    if (argc == 1)
    { // root call

        const char *modes[] = {"call-abort", "div-by-zero"};
        for (size_t i = 0; i < 2; i++)
        {
            const char *args[] = {argv[0], modes[i]};
            int res = 0;
            int exit_code = 0;

            res = subprocess_run(2, args, &exit_code);

            if (res == 0)
            {
                fprintf(stderr, "Subprocess exited with %d instead of crashing.\n", exit_code);
                return 1;
            }
            else if (res < 0)
            {
                fprintf(stderr, "Failed to run subprocess.\n");
                return 1;
            }
        }
        return 0;
    }
    else if (argc == 2)
    { // called by self
        if (strcmp(argv[1], "call-abort") == 0)
        {
            abort();
            return 0;
        }
        else if (strcmp(argv[1], "div-by-zero") == 0)
        {
            volatile int a = 100;
            volatile int b = 5;
            b = 0;
            volatile int c = a / b;
            (void)c;
            return 0;
        }
    }
    fprintf(stderr, "Invalid arguments.\n");
    return 1;
}

int main(int argc, char **argv)
{
    return entry(argc, argv);
}
