/*
    Module for AST data structures
*/
#pragma once
#include <stddef.h>
#include <stdio.h>

typedef struct 
{
    const char *name;
} AstVarDecl;

typedef enum {
    STMT_EXPR,
    STMT_BLOCK,
    STMT_LOOP,
    STMT_IF,
    STMT_VAR_DECL
} AstStmtKind;

typedef struct AstStmt {
    AstStmtKind kind;
    union {
        AstVarDecl var_decl;
        struct {
            size_t stmts_count;
            struct AstStmt* stmts;
        } block;
    } data;
} AstStmt;

typedef struct
{
    const char *name;
    
    size_t args_count;
    AstVarDecl ** args;

    AstStmt* body;
} AstFnDef;

typedef struct
{
    size_t fns_count;
    AstFnDef** fns;
} AstGlobal;

void ast_global_init(AstGlobal*);
void ast_global_deinit(AstGlobal*);

void ast_global_dump(FILE*out, AstGlobal*global);

AstFnDef* ast_global_add_fn(AstGlobal*global);

AstVarDecl* ast_fn_add_arg(AstFnDef*fn);

AstStmt* ast_block_add_stmt(AstStmt*block);
