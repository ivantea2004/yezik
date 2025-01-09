#include "error.h"
#include <stdlib.h>

void error_exit(const char *msg)
{
    if (msg)
    {
        fprintf(stderr, "Internal error: %s\n", msg);
    }
#if YEZIK_DEBUG
    abort();
#else
    exit(1);
#endif
}
