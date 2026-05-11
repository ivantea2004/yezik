#pragma once
#include <stddef.h>

typedef enum
{
    TOKEN_EOF,

    TOKEN_ID,
    TOKEN_INT,
    TOKEN_CHAR,
    TOKEN_STRING,

    TOKEN_NULL,
    TOKEN_UNDEFINED,
    TOKEN_TRUE,
    TOKEN_FALSE,

    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_BREAK,
    TOKEN_CONTINUE,

    TOKEN_FUNCTION,
    TOKEN_CONST,
    TOKEN_TYPE,
    TOKEN_RECORD,

    TOKEN_ASSIGN,

    TOKEN_COMMA,
    TOKEN_COLON,
    TOKEN_SEMI,

    TOKEN_O_PAR,
    TOKEN_C_PAR,
    TOKEN_O_CUR,
    TOKEN_C_CUR,
    TOKEN_O_BR,
    TOKEN_C_BR,

    MAX_TOKEN
} token_kind_t;

#define TOKEN_BUILTINS_X(X)         \
    X(TOKEN_NULL, "null")           \
    X(TOKEN_UNDEFINED, "undefined") \
    X(TOKEN_TRUE, "true")           \
    X(TOKEN_FALSE, "false")

#define TOKEN_KEYWORD_X(X)        \
    X(TOKEN_IF, "if")             \
    X(TOKEN_ELSE, "else")         \
    X(TOKEN_WHILE, "while")       \
    X(TOKEN_BREAK, "break")       \
    X(TOKEN_CONTINUE, "continue") \
                                  \
    X(TOKEN_FUNCTION, "function") \
    X(TOKEN_CONST, "const")       \
    X(TOKEN_TYPE, "type")         \
    X(TOKEN_RECORD, "record")

#define TOKEN_SYMBOLS_X(X) \
    X(TOKEN_ASSIGN, "=")   \
                           \
    X(TOKEN_COMMA, ",")    \
    X(TOKEN_COLON, ":")    \
    X(TOKEN_SEMI, ";")     \
                           \
    X(TOKEN_O_PAR, "(")    \
    X(TOKEN_C_PAR, ")")    \
    X(TOKEN_O_CUR, "{")    \
    X(TOKEN_C_CUR, "}")    \
    X(TOKEN_O_BR, "[")     \
    X(TOKEN_C_BR, "}")

token_kind_t token_parse(const char *p, const char **begin, const char **end);
