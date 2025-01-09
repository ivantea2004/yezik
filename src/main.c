#include <stdio.h>
#include <assert.h>
#include "emitter.h"

static void gen_fn1(EmitCtx *ctx)
{
    emit_fn_begin(ctx, 4, 0, "f1", 1);

    Label loop_begin = emit_ctx_get_unique_label(ctx);
    Label loop_end = emit_ctx_get_unique_label(ctx);
    emit_label(ctx, loop_begin);
    // loop body
    emit_cond_jmp(ctx, loop_end, 0, 1, 1, addr_stack(0), addr_stack(0), 4);

    emit_jmp(ctx, loop_begin);
    emit_label(ctx, loop_end);

    emit_ret(ctx, 4, addr_stack(0));

    emit_fn_end(ctx);
}


static void gen_fn2(EmitCtx *ctx)
{
    emit_fn_begin(ctx, 4, 0, "max", 1);

    /* 
        int max(int a, int b)
        {
            if (a < b) return b;
            return a;
        }
    */

    Label if_end = emit_ctx_get_unique_label(ctx);

    emit_cond_jmp(ctx, if_end, 1, 1, 0, addr_args(0), addr_args(4), 1);
    emit_ret(ctx, 4, addr_args(4));
    emit_label(ctx, if_end);
    emit_ret(ctx, 4, addr_args(0));
    emit_fn_end(ctx);
}

int main()
{
    FILE*out = fopen("out.s", "w");
    assert(out);
    EmitCtx ctx;
    emit_ctx_init(&ctx, out);

    gen_fn1(&ctx);
    gen_fn2(&ctx);

    emit_ctx_deinit(&ctx);
    fclose(out);

    return 0;
}
