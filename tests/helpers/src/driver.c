/*
    Test driver for testing system

    Should be added as a source to test executable

    Expected to be called like
        my_test gen <work_dir> <ctest_include>
            to generate <ctest_include> file
            all tests are run in <work_dir>
        
        my_test run <id>
 */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char**argv)
{
    
    if (argc == 1)
    {
        printf("Running test!\n");
        abort();
        return 1;
    }
    else 
    {
        printf("Running build!\n");
    }
    (void)argc;
    (void)argv;
    return 0;
}
