/*
    Lexer module
*/
#pragma once
#include <stddef.h>
#include <stdint.h>

typedef struct
{
    const char *file;
    const char *text;
    size_t pos;
    size_t prev_pos;
} LexerCtx;

void lexer_ctx_init(LexerCtx *ctx, const char *file, const char *text);
void lexer_ctx_deinit(LexerCtx *ctx);

typedef enum
{
    TOKEN_CHAR, // char literal
    TOKEN_STR,  // string literal
    TOKEN_INT,  // int literal
    TOKEN_ID,   // identifier

    // keywords
    TOKEN_FN,
    TOKEN_LET,
    TOKEN_CONST,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_LOOP,
    TOKEN_BREAK,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,

    // brackets
    TOKEN_PAR_OPEN,   // '('
    TOKEN_PAR_CLOSED, // ')'
    TOKEN_BR_OPEN,    // '['
    TOKEN_BR_CLOSED,  // ']'
    TOKEN_CUR_OPEN,   // '{'
    TOKEN_CUR_CLOSED, // '}'

    // control
    TOKEN_COMMA,
    TOKEN_SEMI,
    TOKEN_COLON,

    // operators
    TOKEN_DOT,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULT,
    TOKEN_DIV,
    TOKEN_REM,
    TOKEN_CMP_E,
    TOKEN_CMP_NE,
    TOKEN_CMP_L,
    TOKEN_CMP_G,
    TOKEN_CMP_LE,
    TOKEN_CMP_GE,
    TOKEN_BIT_AND,
    TOKEN_BIT_OR,
    TOKEN_BIT_NOT

} TokenType;

typedef struct
{
    TokenType type;
    const char *file;
    const char *text;
    size_t begin;
    size_t end;
} Token;

/*
    If EOF returns > 0
    If error return <0
    On ok returns 0
*/
int lexer_peek(LexerCtx *ctx, Token *token);

/*
    Moves lexer one token back
*/
void lexer_unget(LexerCtx *ctx);

/*
    If EOF or error returns != 0
    On ok returns 0
*/
int lexer_get(LexerCtx *ctx, Token *token);
