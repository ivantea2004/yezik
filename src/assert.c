#include <yezik/assert.h>
#include <stdlib.h>

void yezik_panic()
{
#if YEZIK_DEBUG
    abort();
#else
    exit(1);
#endif
}
