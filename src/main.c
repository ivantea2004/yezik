#include <stdio.h>
#include "compiler.h"

int main(int argc, char **argv)
{
#ifdef NDEBUG
    if (argc < 3)
    {
        fprintf(stderr, "Run 'yezik <src> <out>' to compile file.\n");
        return 1;
    }
    return compile_file(argv[1], argv[2]);
#else
    (void)argc;
    (void)argv;
    return compile_file("test.yez", "test.s");
#endif
}
