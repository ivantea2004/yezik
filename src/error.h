/*

    Error handling and reporting module of compiler

 */
#pragma once
#include <stddef.h>
#include <stdio.h>

#ifndef NDEBUG
#define YEZIK_DEBUG 1
#endif

/*
    Should be used to report internal errors
    if <msg> is not NULL, then prints
    Internal error: <msg> to stderr
    Finishes program with exit(1) or abort()
*/
void error_exit(const char *msg);

#if YEZIK_DEBUG
#include <stdio.h>
#include <inttypes.h>

#define YEZIK_ASSERT(expect_true, ...)                                                       \
    if (!(expect_true))                                                                      \
    {                                                                                        \
        fprintf(stderr, "Internal error at %s:%" PRIuPTR ":\n", __FILE__, (size_t)__LINE__); \
        fprintf(stderr, __VA_ARGS__);                                                        \
        error_exit(NULL);                                                                    \
    }
#else

#define YEZIK_ASSERT(expect_true, ...) \
    if (!(expect_true))                \
    {                                  \
        error_exit(NULL);              \
    }

#endif

#define YEZIK_UNREACHABLE(msg) \
    YEZIK_ASSERT(0, (msg))

/*
    Prints a line containing <place> and hightlights <place>
*/
void error_snippet(FILE *out, const char *text, size_t place);

/*
    Finds line which contains <place>
    In <begin> and <end> line borders are written
    Returns line's number starting from 0
*/
size_t error_find_line(const char *text, size_t place, size_t *begin, size_t *end);
