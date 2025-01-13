#include "error.h"
#include <stdlib.h>
#include <inttypes.h>

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

static size_t digits_count_(size_t n)
{
    size_t c = 0;
    do
    {
        c++;
        n /= 10;
    } while (n > 0);
    return c;
}

static void print_n_(FILE *out, char c, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        fputc(c, out);
    }
}

enum
{
    HIGHLIGHT_WIDTH = 2
};

void error_snippet(FILE *out, const char *text, size_t place)
{
    size_t begin, end;
    size_t number = error_find_line(text, place, &begin, &end);
    size_t line_place = place - begin;
    number++;
    size_t number_width = digits_count_(number);
    fprintf(out, " %" PRIuPTR " |", number);
    for (size_t i = 0; i < end - begin; i++)
    {
        fputc(text[begin + i], out);
    }
    fprintf(out, "\n");
    print_n_(out, ' ', number_width + 2);
    fprintf(out, "|");
    for (size_t i = 0;; i++)
    {
        if (i + HIGHLIGHT_WIDTH < line_place)
        {
            fputc(' ', out);
        }
        else if (i + HIGHLIGHT_WIDTH >= line_place && i < line_place)
        {
            fputc('~', out);
        }
        else if (i == line_place)
        {
            fputc('^', out);
            break;
        }
    }
    print_n_(out, '~', HIGHLIGHT_WIDTH);
    fprintf(out, "\n");
}

size_t error_find_line(const char *text, size_t place, size_t *begin, size_t *end)
{
    size_t curr_line_number = 0;
    size_t curr_begin = 0;
    size_t i = 0;
    while (1)
    {
        if (text[i] == '\n')
        {
            // last line
            if (text[i + 1] == '\0')
            {
                YEZIK_ASSERT(place < i + 1, "Index out of bounds.\n");
                if (begin)*begin = curr_begin;
                if (end)*end = i;
                return curr_line_number;
            }
            else
            {
                if (place <= i)
                {
                    if (begin)*begin = curr_begin;
                    if (end)*end = i;
                    return curr_line_number;
                }
                else
                {
                    curr_line_number++;
                    curr_begin = i + 1;
                }
            }
        } else if(text[i] == '\0') {
            if (begin) *begin = curr_begin;
            if (end) *end = i;
            return curr_line_number;
        }
        i++;
    }
}
