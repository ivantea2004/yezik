#pragma once

#ifndef YEZIK_BUILD_TESTING
#define YEZIK_BUILD_TESTING 0
#endif

#define YEZIK_TEST(name, ...) \
    int name()

#define YEZIK_TEST_HEADER_DECLARE_TEST(name, ...) \
    extern int name();

#define YEZIK_TEST_HEADER_GET_TEST_FN_NAME(name, ...) \
    name

typedef int (*YezikTestCaseFn)();
