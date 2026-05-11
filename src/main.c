#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "import.h"

int main(int argc, char **argv)
{

    if (argc != 3)
    {
        fprintf(stderr, "error: Expected arguments [in].yezik [out].c\n");
        exit(1);
    }

    errno = 0;
    FILE *out = fopen(argv[2], "w");
    if (!out)
    {
        fprintf(stderr, "error: %s: %s\n", argv[2], strerror(errno));
        exit(1);
    }

    import_file(argv[1], out);
    fclose(out);
    return 0;
}
