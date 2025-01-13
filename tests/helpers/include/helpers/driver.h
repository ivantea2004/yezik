#pragma once

typedef struct {
    void(*test)();
    const char*test_name;
    int should_fail;
} TestInfo;

typedef TestInfo (*TestFunction)();
