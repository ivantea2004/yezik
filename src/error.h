#pragma once
#include <stdio.h>
#include <stdlib.h>

#ifdef NDEBUG
#define terminate() exit(1)
#else
#define terminate() (*(char *)0 = 0, exit(1))
#endif

void print_location(FILE *stream, const char *place);
void print_snippet(FILE *stream, const char *begin, const char *end);
