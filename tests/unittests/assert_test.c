#include <tests/test_config.h>
#include <src/assert.c>

TEST(Panic, should_fail = 1)
{
    YEZIK_PANIC("This should fail\n");
}

TEST(Assert_Pass, )
{
    YEZIK_ASSERT(0 == 0, "Expecting true\n");
}

TEST(Assert_Fail, )
{
    YEZIK_ASSERT(1 == 0, "Expecting true\n");
}
