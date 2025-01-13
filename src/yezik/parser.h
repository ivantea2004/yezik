/*
    Parser module
*/
#pragma once
#include "lexer.h"
#include "ast.h"

int parse_ast_global(LexerCtx*lexer, AstGlobal*global);
