#pragma once

typedef enum
{
    TOKEN_EOF,

    TOKEN_ID,
    TOKEN_INT,
    TOKEN_CHAR,
    TOKEN_STRING,

    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_BREAK,
    TOKEN_CONTINUE,

    TOKEN_FUNCTION,
    TOKEN_CONST,
    TOKEN_TYPE,
    TOKEN_RECORD,

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

token_kind_t token_parse(const char *p, const char **begin, const char **end);
