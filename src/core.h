#pragma once
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    char **strs;
    size_t size;
} str_list_t;

const char *str_list_find(const str_list_t *list, const char *str);
void str_list_append(str_list_t *list, const char *str);
void str_list_clear(str_list_t *list);

void path_normalize(char *path);

#ifdef NDEBUG
#define terminate() exit(1)
#else
#define terminate() (*(char *)0 = 0, exit(1))
#endif

void print_location(FILE *stream, const char *place);
void print_snippet(FILE *stream, const char *begin, const char *end);

void import_file(const char *path, FILE *out, const char *import_begin, const char *import_end);
