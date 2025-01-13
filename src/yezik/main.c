#include <stdio.h>
//#include "compiler.h"
#include <yezik/test.h>

int handle_args(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Run 'yezik <src> <out>' to compile file.\n");
        return 1;
    }
    //return compile((size_t)argc - 2, (const char**)argv + 1, argv[argc - 1]);
    (void)argv;
    return 0;
}

#if !YEZIK_BUILD_TESTING
int main(int argc, char **argv)
{

#ifdef NDEBUG
    return handle_args(argc - 1, argv + 1);
#else
    (void)argc;
    (void)argv;
    char *in[] = {"test.yez", "test.s"};
    return handle_args(2, in);
#endif
}
#endif
