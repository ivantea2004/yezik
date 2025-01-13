/*
    First four defines are used to discover tests
*/
#pragma once
#include <helpers/driver.h>
#define TEST(name, ...)              \
    static void name##_test();       \
    TestInfo name##_test_interface() \
    {                                \
        TestInfo i;                  \
        i.test = name##_test;        \
        i.test_name = #name;         \
        int should_fail = 0;         \
        __VA_ARGS__;                 \
        i.should_fail = should_fail; \
        return i;                    \
    }                                \
    void name##_test()

#define TEST_DECL_FUNC(name, ...) TestInfo name##_test_interface();
#define TEST_GET_FUNC(name, ...) name##_test_interface
#define TEST_FUNC_TYPE TestFunction
