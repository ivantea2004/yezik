#include "generator.h"

static int gen_ast_fn_def_(EmitCtx *emitter, AstFnDef *fn)
{
    emit_fn_begin(emitter, 0, 0, fn->name, 1);
    emit_fn_end(emitter);
    return 0;
}

int gen_ast_global(EmitCtx *emitter, AstGlobal *global)
{
    for (size_t i = 0; i < global->fns_count; i++)
    {
        if (gen_ast_fn_def_(emitter, global->fns[i]))
            return 1;
    }
    return 0;
}
