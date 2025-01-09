#include "ast.h"
#include <stdlib.h>

static void ast_fn_def_init_(AstFnDef *fn);
static void ast_fn_def_deinit_(AstFnDef *fn);

void ast_global_init(AstGlobal *global)
{
    global->fns_count = 0;
    global->fns = NULL;
}
void ast_global_deinit(AstGlobal *global)
{
    for (size_t i = 0; i < global->fns_count; i++)
    {
        ast_fn_def_deinit_(global->fns[i]);
        free(global->fns[i]);
    }
}

static void ast_fn_def_dump_(FILE *out, AstFnDef *fn)
{
    fprintf(out, "fn %s()\n", fn->name);
}

void ast_global_dump(FILE *out, AstGlobal *global)
{
    for (size_t i = 0; i < global->fns_count; i++)
    {
        ast_fn_def_dump_(out, global->fns[i]);
    }
}

AstFnDef *ast_global_add_fn(AstGlobal *global)
{
    global->fns = realloc(global->fns, sizeof(AstFnDef *) * (global->fns_count + 1));
    global->fns[global->fns_count] = malloc(sizeof(AstFnDef));
    ast_fn_def_init_(global->fns[global->fns_count]);
    global->fns_count++;
    return global->fns[global->fns_count - 1];
}

static void ast_fn_def_init_(AstFnDef *fn)
{
    fn->args_count = 0;
}

static void ast_fn_def_deinit_(AstFnDef *fn)
{
    (void)fn;
}
