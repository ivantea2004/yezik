#include "lexer.h"
#include "core.h"
#include <string.h>

int is_space(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

int is_digit(char c)
{
    return '0' <= c && c <= '9';
}

int is_id_char(char c)
{
    return is_digit(c) || ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || c == '_';
}

token_kind_t token_parse(const char *p, const char **begin, const char **end)
{

#define KEYWORD_MATCH(kind, str)                                                        \
    if (*end - *begin == sizeof(str) - 1 && strncmp(str, *begin, sizeof(str) - 1) == 0) \
        return kind;

#define SYMBOL_MATCH(kind, str)      \
    do                               \
    {                                \
        const char *i = str;         \
        const char *q = p;           \
        while (*i && *q && *i == *q) \
            i++, q++;                \
        if (*i)                      \
            break;                   \
        *end = q;                    \
        return kind;                 \
    } while (0);

    const char *expected;
    while (*p && is_space(*p))
        p++;

    *begin = p;

    if (!*p)
    {
        *end = p;
        return TOKEN_EOF;
    }
    else if (is_digit(*p))
    {
        while (*p && is_digit(*p))
            p++;
        *end = p;
        return TOKEN_INT;
    }
    else if (is_id_char(*p))
    {
        while (*p && is_id_char(*p))
            p++;
        *end = p;

        TOKEN_BUILTINS_X(KEYWORD_MATCH);
        TOKEN_KEYWORD_X(KEYWORD_MATCH);

        return TOKEN_ID;
    }
    else if (*p == '"')
    {
        *begin = p;
        p++;
        while (*p && *p != '"')
            p++;
        if (*p == '"')
        {
            *end = p + 1;
            return TOKEN_STRING;
        }
        else
        {
            expected = "matching '\"'";
            goto unexpected_eof;
        }
    }
    else
    {
        TOKEN_SYMBOLS_X(SYMBOL_MATCH);

        print_location(stderr, p);
        fprintf(stderr, "error: Unexpected char `%c`.\n", *p);
        print_snippet(stderr, p, p + 1);
        terminate();
    }

unexpected_eof:
    print_location(stderr, p);
    fprintf(stderr, "error: Unexpected EOF. Expected %s \n", expected);
    print_snippet(stderr, p - 1, p);
    terminate();

#undef HARDCODED
#undef KEYWORD
}
