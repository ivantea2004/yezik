/*
    Code generator module
*/
#pragma once
#include "ast.h"
#include "emitter.h"

int gen_ast_global(EmitCtx*emitter, AstGlobal*global);
