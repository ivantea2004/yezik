#pragma once

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>

#ifndef NDEBUG
#define YEZIK_DEBUG 1
#endif

void yezik_panic();

/*
    Should be used to report internal (logic) errors
 */
#define YEZIK_PANIC(...)                                                                     \
    {                                                                                        \
        fprintf(stderr, "Internal error at %s:%" PRIuPTR ":\n", __FILE__, (size_t)__LINE__); \
        fprintf(stderr, __VA_ARGS__);                                                        \
        yezik_panic();                                                                       \
    }

#define YEZIK_ASSERT(expect_true, ...) \
    if (!(expect_true))                \
    {                                  \
        YEZIK_PANIC(__VA_ARGS__);      \
    }
