#include <yezik/yezik.h>
#include <yezik/test.h>
#include <stdlib.h>

void yezik_panic()
{
#if YEZIK_DEBUG
    abort();
#else
    exit(1);
#endif
}

YEZIK_TEST(test1, "")
{
    printf("in test 1\n");
    char*ptr = malloc(1024ull*1024*1024 * 1024);
    *ptr = 10;
    return 0;
}
