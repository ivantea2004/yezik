/*

    Simple header only library to run subprocesses

    define macro
    #define SUBPROCESS_IMPL
    to get implementations

 */
#pragma once
#include <stddef.h>

/*
    Runs specified command.
    Negative value is returned if error has happended during startup.
    Positive value is returned if process did not exit properly.
    Zero is returned if process exited properly, exit code is returned
*/
int subprocess_run(int argc, const char **argv, int *exit_code);

#ifdef SUBPROCESS_IMPL

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int subprocess_run(int argc, const char **argv, int *exit_code)
{
    (void)exit_code;
    (void)argv;
    assert(argc > 0);

    int res = fork();

    if (res == -1)
    {
        perror("fork");
        return -1;
    }

    if (res == 0)
    {
        const char **args = calloc((size_t)argc + 1, sizeof(char *));
        memcpy(args, argv, (size_t)argc * sizeof(char *));
        execv(args[0], (char*const*)args);
        perror("exec");
        exit(1);
    }
    else
    {
        
        int child_pid = res;
        int status;
        if (waitpid(child_pid, &status, 0) != child_pid)
        {
            perror("waitpid");
            return -1;
        }

        if (WIFEXITED(status))
        {
            *exit_code = WEXITSTATUS(status);
            return 0;
        }
        else
        {
            return 1;
        }
    }
}

#endif
