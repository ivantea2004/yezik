#include "emitter.h"
#include "error.h"
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
    YEZIK_ASSERT(!ctx->in_fn, "Can not destroy emitting context while in function.\n");
}

#define EMIT_IMPL(...) fprintf((ctx)->out, __VA_ARGS__)

#define EMIT_GLOBAL(...)                                                                           \
    {                                                                                              \
        YEZIK_ASSERT(!(ctx)->in_fn, "This macro is expected to be used outside of a function.\n"); \
        EMIT_IMPL(__VA_ARGS__);                                                                    \
    }

#define EMIT_FN(...)                                                                          \
    {                                                                                         \
        YEZIK_ASSERT((ctx)->in_fn, "This macro is expected to be used inside a function.\n"); \
        EMIT_IMPL(__VA_ARGS__);                                                               \
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
    YEZIK_ASSERT(ctx->in_fn, "Label are only allowed in functions.\n");
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
    REG_EAX,
    REG_EBX,
    REG_ECX,
    REG_EDX,
    REG_EDI,
    REG_ESI
} Reg;

static void emit_reg_(EmitCtx *ctx, Reg r, size_t size)
{
    if (r == REG_EDI)
    {
        YEZIK_ASSERT(size == 4, "EDI can only be 4 bytes.\n");
        EMIT("edi");
    }
    else if (r == REG_ESI)
    {
        YEZIK_ASSERT(size == 4, "ESI can only be 4 bytes.\n");
        EMIT("esi");
    }
    else
    {
        YEZIK_ASSERT(size == 1 || size == 2 || size == 4, "Registers can only be 1, 2 or 4 bytes.\n");
        if (size == 4)
        {
            EMIT("e");
        }
        switch (r)
        {
        case REG_EAX:
            EMIT("a");
            break;
        case REG_EBX:
            EMIT("b");
            break;
        case REG_ECX:
            EMIT("c");
            break;
        case REG_EDX:
            EMIT("d");
            break;
        default:
            YEZIK_UNREACHABLE("Undefined register.\n");
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
        YEZIK_UNREACHABLE("Invalid deref size.\n");
    }
}

static void emit_addr_(EmitCtx *ctx, Addr addr)
{
    if (addr.type == ADDR_VAR)
    {
        YEZIK_UNREACHABLE("Global variables are not implemented yet.\n");
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
            YEZIK_UNREACHABLE("Undefined addr type.\n");
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
        YEZIK_ASSERT(ret_value.type == ADDR_NONE, "If return_size is 0 then addr must be NONE");
    }
    else
    {
        EMIT_LOAD(ret_size, REG_EAX, ret_value);
    }
    EMIT("jmp .ret\n");
}

void emit_label(EmitCtx *ctx, Label label)
{
    EMIT(".%" PRIuPTR ":\n", label);
}

void emit_jmp(EmitCtx *ctx, Label label)
{
    YEZIK_ASSERT(label < ctx->max_label, "Invalid label value.\n");
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
    YEZIK_ASSERT(order != 0 || (order == 0 && eq), "If comparation is no < or > then it must be ==\n");
    YEZIK_ASSERT(order != 0 || (order == 0 && sign), "Signed comparation is only allowed for < or >.\n");
    YEZIK_ASSERT(label < ctx->max_label, "Invalid label value.\n");
    EMIT_LOAD(size, REG_EAX, left);
    EMIT_LOAD(size, REG_ECX, right);
    EMIT("cmp ");
    EMIT_REG(REG_EAX, size);
    EMIT(", ");
    EMIT_REG(REG_ECX, size);
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
