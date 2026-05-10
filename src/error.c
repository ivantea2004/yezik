#include "error.h"
#include "import.h"

void find_line(const char *place, const char **begin, const char **end, size_t *line_number)
{
    const char *i = current_source_file_text();
    *line_number = 1;
    *begin = i;

    for (; *i; i++)
        if (*i == '\n')
        {
            if (place <= i)
            {
                *end = i;
                return;
            }
            else
            {
                *begin = i + 1;
                (*line_number)++;
            }
        }
    *end = i;
}

void print_location(FILE *stream, const char *place)
{
    const char *line_begin;
    const char *line_end;
    size_t line_number;
    find_line(place, &line_begin, &line_end, &line_number);
    fprintf(stream, "%s:%d:%d: ", current_source_file_path(), (int)line_number, (int)(place - line_begin) + 1);
}

void print_snippet(FILE *stream, const char *begin, const char *end)
{
    const char *line_begin;
    const char *line_end;
    size_t line_number;
    find_line(begin, &line_begin, &line_end, &line_number);
    if (end >= line_end)
        end = line_end;
    size_t offset = fprintf(stream, " %d ", (int)line_number);
    fprintf(stream, "| %.*s\n", (int)(line_end - line_begin), line_begin);
    if (end > begin)
    {
        fprintf(stream, "%*s| ", (int)offset, "");
        for (const char *i = line_begin; i < end; i++)
            fputc(i < begin ? ' ' : i == begin ? '^'
                                               : '~',
                  stream);
        fputc('\n', stream);
    }
}
