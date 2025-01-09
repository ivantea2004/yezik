#include "emitter.h"
#include <assert.h>
#include <inttypes.h>

enum
{
    ARCH_SIZE = 4
};

void emit_ctx_init(EmitCtx *ctx, FILE *out)
{
    ctx->out = out;
    ctx->in_fn = 0;
    ctx->fn_stack_size = 0;
    ctx->fn_call_size = 0;
    ctx->max_label = 0;
}
void emit_ctx_deinit(EmitCtx *ctx)
{
    assert(!ctx->in_fn);
}

#define EMIT_IMPL(...) fprintf((ctx)->out, __VA_ARGS__)

#define EMIT_GLOBAL(...)        \
    {                           \
        assert(!(ctx)->in_fn);  \
        EMIT_IMPL(__VA_ARGS__); \
    }

#define EMIT_FN(...)            \
    {                           \
        assert((ctx)->in_fn);   \
        EMIT_IMPL(__VA_ARGS__); \
    }

#define EMIT(...) EMIT_FN(__VA_ARGS__)

void emit_fn_begin(
    EmitCtx *ctx,
    size_t stack_size,
    size_t call_size,
    const char *fn_name,
    int is_global)
{
    stack_size = (stack_size + ARCH_SIZE - 1) / ARCH_SIZE * ARCH_SIZE;
    call_size = (call_size + ARCH_SIZE - 1) / ARCH_SIZE * ARCH_SIZE;
    if (is_global)
    {
        EMIT_GLOBAL("global %s\n", fn_name);
    }
    EMIT_GLOBAL("section .text\n");
    EMIT_GLOBAL("%s:\n", fn_name);
    ctx->fn_stack_size = stack_size;
    ctx->fn_call_size = call_size;
    ctx->max_label = 0;
    ctx->in_fn = 1;
    EMIT("push ebp\n");
    EMIT("push ebx\n");
    EMIT("push edi\n");
    EMIT("push esi\n");
    EMIT("mov ebp, esp\n");
    EMIT("sub esp, %" PRIuPTR "\n", stack_size + call_size);
}

void emit_fn_end(EmitCtx *ctx)
{
    EMIT(".ret:\n")
    EMIT("add esp, %" PRIuPTR "\n", ctx->fn_stack_size + ctx->fn_call_size);
    EMIT("pop esi\n");
    EMIT("pop edi\n");
    EMIT("pop ebx\n");
    EMIT("pop ebp\n");
    EMIT("ret\n");
    ctx->in_fn = 0;
}

Label emit_ctx_get_unique_label(EmitCtx *ctx)
{
    assert(ctx->in_fn);
    return ctx->max_label++;
}

Addr addr_none()
{
    Addr a;
    a.type = ADDR_NONE;
    return a;
}
static Addr addr_(AddrType type, size_t offset)
{
    Addr a;
    a.type = type;
    a.value.offset = offset;
    return a;
}
Addr addr_stack(size_t offset)
{
    return addr_(ADDR_STACK, offset);
}
Addr addr_args(size_t offset)
{
    return addr_(ADDR_ARGS, offset);
}
Addr addr_call(size_t offset)
{
    return addr_(ADDR_CALL, offset);
}
Addr addr_var(const char *var_name)
{
    Addr a;
    a.type = ADDR_VAR;
    a.value.var_name = var_name;
    return a;
}

typedef enum
{
    REG_A,
    REG_B,
    REG_C,
    REG_D,
    REG_EDI,
    REG_ESI
} Reg;

static void emit_reg_(EmitCtx *ctx, Reg r, size_t size)
{
    if (r == REG_EDI)
    {
        assert(size == 4);
        EMIT("edi");
    }
    else if (r == REG_ESI)
    {
        assert(size == 4);
        EMIT("esi");
    }
    else
    {
        assert(size == 1 || size == 2 || size == 4);
        if (size == 4)
        {
            EMIT("e");
        }
        switch (r)
        {
        case REG_A:
            EMIT("a");
            break;
        case REG_B:
            EMIT("b");
            break;
        case REG_C:
            EMIT("c");
            break;
        case REG_D:
            EMIT("d");
            break;
        default:
            assert(0);
        }
        if (size > 1)
        {
            EMIT("x");
        }
        if (size == 1)
        {
            EMIT("l");
        }
    }
}

static void emit_deref_size_(EmitCtx *ctx, size_t size)
{
    switch (size)
    {
    case 1:
        EMIT("byte");
        break;
    case 2:
        EMIT("word");
        break;
    case 4:
        EMIT("dword");
        break;
    default:
        assert(0);
    }
}

static void emit_addr_(EmitCtx *ctx, Addr addr)
{
    if (addr.type == ADDR_VAR)
    {
        assert(0);
    }
    else
    {
        int64_t offset = (int64_t)addr.value.offset;
        switch (addr.type)
        {
        case ADDR_STACK:
            offset -= ctx->fn_stack_size;
            break;
        case ADDR_CALL:
            offset -= ctx->fn_stack_size + ctx->fn_call_size;
            break;
        case ADDR_ARGS:
            offset += 20;
            break;
        default:
            assert(0);
        }
        if (offset < 0)
        {
            EMIT("ebp - %" PRIi64, -offset);
        }
        else
        {
            EMIT("ebp + %" PRIi64, offset);
        }
    }
}

#define EMIT_REG(r, size) emit_reg_((ctx), (r), (size))
#define EMIT_ADDR(addr) emit_addr_((ctx), (addr))
#define EMIT_DEREF(addr, size)           \
    {                                    \
        emit_deref_size_((ctx), (size)); \
        EMIT(" [");                      \
        EMIT_ADDR((addr));               \
        EMIT("]");                       \
    }

#define EMIT_LOAD(size, r, addr)    \
    {                               \
        EMIT("mov ");               \
        EMIT_REG((r), (size));      \
        EMIT(", ");                 \
        EMIT_DEREF((addr), (size)); \
        EMIT("\n");                 \
    }

void emit_ret(EmitCtx *ctx, size_t ret_size, Addr ret_value)
{
    if (ret_size == 0)
    {
        assert(ret_value.type == ADDR_NONE);
    }
    else
    {
        EMIT_LOAD(ret_size, REG_A, ret_value);
    }
    EMIT("jmp .ret\n");
}

void emit_label(EmitCtx *ctx, Label label)
{
    EMIT(".%" PRIuPTR ":\n", label);
}

void emit_jmp(EmitCtx *ctx, Label label)
{
    EMIT("jmp .%" PRIuPTR "\n", label);
}
void emit_cond_jmp(
    EmitCtx *ctx,
    Label label,
    int order,
    int eq,
    int sign,
    Addr left,
    Addr right,
    size_t size)
{
    EMIT_LOAD(size, REG_A, left);
    EMIT_LOAD(size, REG_C, right);
    EMIT("cmp ");
    EMIT_REG(REG_A, size);
    EMIT(", ");
    EMIT_REG(REG_C, size);
    EMIT("\n");
    EMIT("j");
    if (order < 0 && sign)
    {
        EMIT("l");
    }
    else if (order > 0 && sign)
    {
        EMIT("g");
    }
    else if (order < 0 && !sign)
    {
        EMIT("b");
    }
    else if (order > 0 && !sign)
    {
        EMIT("a");
    }
    if (eq)
    {
        EMIT("e");
    }
    EMIT(" .%" PRIuPTR "\n", label);
}
