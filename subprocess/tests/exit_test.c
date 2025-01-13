
#define SUBPROCESS_IMPL
#include <subprocess/subprocess.h>
#include <stdio.h>

enum {
    MAX = 100
};

static int parent(const char*self)
{
    for (int i = 0; i < MAX; i++)
    {
        char buff[100];
        snprintf(buff, sizeof(buff), "%d", i);
        const char* argv[] = {self, buff};
        int exit_code = 0;
        int res = subprocess_run(2, argv, &exit_code);
        if (res > 0)
        {
            fprintf(stderr, "Subprocess crashed.\n");
            return 1;
        } else if(res < 0)
        {
            fprintf(stderr, "Faied to run subprocess.\n");
        }
        if (exit_code != i)
        {
            fprintf(stderr, "Subprocess returned incorrect value, original = %d, exit code = %d.\n", i, exit_code);
            return 1;
        }
    }
    return 0;    
}

static int child(int n)
{
    printf("Got number %d\n", n);
    return n;
}

static int entry(int argc, char **argv)
{
    if (argc == 1){ // root call
        return parent(argv[0]);
    } else if(argc == 2) { // called by self
        int x;
        if (sscanf(argv[1], "%d", &x) == 1)
        {
            return child(x);
        }
    }
    fprintf(stderr, "Invalid arguments.\n");
    return 1;
}

int main(int argc, char**argv)
{
    return entry(argc, argv);
}
