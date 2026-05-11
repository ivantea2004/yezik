#include "core.h"
#include "parser.h"
#include <string.h>

const char *str_list_find(const str_list_t *list, const char *str)
{
    for (size_t i = 0; i < list->size; i++)
        if (strcmp(str, list->strs[i]) == 0)
            return list->strs[i];
    return NULL;
}

void str_list_append(str_list_t *list, const char *str)
{
    list->strs = realloc(list->strs, sizeof(list->strs) * (list->size + 1));
    list->strs[list->size] = strdup(str);
    list->size++;
}

void str_list_clear(str_list_t *list)
{
    for (size_t i = 0; i < list->size; i++)
        free(list->strs[i]);
    free(list->strs);
    list->size = 0;
}

void path_normalize(char *path)
{
    const char *read = path;
    char *write = path;
    for (; *read; write++, read++)
    {
        if (read[0] == '.' && read[1] == '/')
        {
            read++;
            write--;
            continue;
        }
        else if (read[0] == '.' && read[1] == '.' && read[2] == '/')
        {
            read++;
            read++;
            while (*write != '/')
                write--;
            write--;
            while (write >= path && *write != '/')
                write--;
            continue;
        }
        else
        {
            for (; *read && *read != '/'; read++, write++)
                *write = *read;
            *write = *read;
        }
    }
}

static char *current_source_file_path;
static char *current_source_file_text;

void find_location(const char **place, const char **begin, const char **end, size_t *line_number)
{
    while (**place == ' ' || **place == '\t' || **place == '\r' || **place == '\n')
        (*place)++;

    if (!**place)
        (*place)--;

    const char *i = current_source_file_text;
    *line_number = 1;
    *begin = i;

    for (; *i; i++)
        if (*i == '\n')
        {
            if (*place <= i)
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
    find_location(&place, &line_begin, &line_end, &line_number);
    fprintf(stream, "%s:%d:%d: ", current_source_file_path, (int)line_number, (int)(place - line_begin) + 1);
}

void print_snippet(FILE *stream, const char *begin, const char *end)
{
    const char *line_begin;
    const char *line_end;
    size_t line_number;
    find_location(&begin, &line_begin, &line_end, &line_number);
    if (end <= begin)
        end = begin + 1;
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

static str_list_t imported_paths;

void import_file(const char *path_, FILE *out, const char *import_begin, const char *import_end)
{
    char *path;
    if (current_source_file_path)
    {
        char *buff = calloc(strlen(current_source_file_path) + 4 + strlen(path_) + 1, 1);
        strcat(buff, current_source_file_path);
        strcat(buff, "/../");
        strcat(buff, path_);
        path = buff;
    }
    else
    {
        path = strdup(path_);
    }
    path_normalize(path);

    if (str_list_find(&imported_paths, path))
        return;

    char *tmp_path = current_source_file_path;
    char *tmp_text = current_source_file_text;

    errno = 0;
    FILE *file = fopen(path, "rb");
    if (!file)
    {
        if (import_begin)
            print_location(stderr, import_begin);
        fprintf(stderr, "error: %s: %s.\n", path, strerror(errno));
        if (import_begin)
            print_snippet(stderr, import_begin, import_end);
        terminate();
    }
    fseek(file, 0, SEEK_END);
    size_t input_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    current_source_file_text = calloc(input_size + 1, 1);
    if (fread(current_source_file_text, 1, input_size, file) < input_size)
    {
        fprintf(stderr, "error: %s: %s.\n", path, strerror(errno));
        exit(1);
    }
    fclose(file);

    str_list_append(&imported_paths, path);
    current_source_file_path = path;
    parse_file(current_source_file_text, out);

    current_source_file_path = tmp_path;
    current_source_file_text = tmp_text;
}
