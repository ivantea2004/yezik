#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

/*
    EmitCtx
*/

typedef struct
{
    FILE *out;
    int in_fn;
    size_t fn_stack_size;
    size_t fn_call_size;
    size_t max_label;
} EmitCtx;

void emit_ctx_init(EmitCtx *ctx, FILE *out);
void emit_ctx_deinit(EmitCtx *ctx);

void emit_fn_begin(
    EmitCtx *ctx,
    size_t stack_size,
    size_t call_size,
    const char *fn_name,
    int is_global);
void emit_fn_end(EmitCtx *ctx);

typedef size_t Label;
Label emit_ctx_get_unique_label(EmitCtx *ctx);

/*
    Addr
*/

typedef enum
{
    ADDR_NONE,
    ADDR_STACK,
    ADDR_ARGS,
    ADDR_CALL,
    ADDR_VAR
} AddrType;

typedef struct
{
    AddrType type;
    union
    {
        const char *var_name;
        size_t offset;
    } value;
} Addr;

Addr addr_none();
Addr addr_stack(size_t offset);
Addr addr_args(size_t offset);
Addr addr_call(size_t offset);
Addr addr_var(const char *var_name);

/*
    Instructions
*/

// function call
void emit_call(EmitCtx *ctx, const char *fn_name, size_t ret_size, Addr ret_value);

// return
void emit_ret(EmitCtx *ctx, size_t ret_size, Addr ret_value);

// a = 10
void emit_store_const(EmitCtx *ctx, Addr dst, int64_t value);

// pa = &a
void emit_store_addr(EmitCtx *ctx, Addr dst, Addr src);

// b = *pa
void emit_load(EmitCtx *ctx, Addr dst, Addr src);

// *pc = b
void emit_store(EmitCtx *ctx, Addr dst, Addr src);

// brancing
void emit_label(EmitCtx *ctx, Label label);
void emit_jmp(EmitCtx *ctx, Label label);
void emit_cond_jmp(
    EmitCtx *ctx,
    Label label,
    int order,
    int eq,
    int sign,
    Addr left,
    Addr right,
    size_t size);
