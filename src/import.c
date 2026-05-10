#include "import.h"
#include "parse.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>

static const char *path;
static char *text;

const char *current_source_file_path()
{
    return path;
}

const char *current_source_file_text()
{
    return text;
}

void import_file(const char *path_, FILE *out)
{
    path = path_;

    errno = 0;
    FILE *file = fopen(path, "rb");
    if (!file)
    {
        fprintf(stderr, "error: %s: %s.\n", path, strerror(errno));
        perror(path);
        exit(1);
    }
    fseek(file, 0, SEEK_END);
    size_t input_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    text = calloc(input_size + 1, 1);
    if (fread(text, 1, input_size, file) < input_size)
    {
        fprintf(stderr, "error: %s: %s.\n", path, strerror(errno));
        exit(1);
    }
    fclose(file);

    parse_file(out);
}
