#include "helpers.h"
#include "core.h"
#include <stdlib.h>
#include <string.h>

const char *token_kind_str(token_kind_t kind)
{

#define KEYWORD_CASE(kind, str) \
    case kind:                  \
        return "`" str "` keyword";

#define SYMBOL_CASE(kind, str) \
    case kind:                 \
        return "`" str "`";

    switch (kind)
    {
    case TOKEN_EOF:
        return "EOF";
    case TOKEN_ID:
        return "identifier";
    case TOKEN_INT:
        return "integer literal";
    case TOKEN_CHAR:
        return "character literal";
    case TOKEN_STRING:
        return "string literal";
        TOKEN_BUILTINS_X(SYMBOL_CASE);
        TOKEN_KEYWORD_X(KEYWORD_CASE);
        TOKEN_SYMBOLS_X(SYMBOL_CASE);
    default:
        fprintf(stderr, "internal: Unknown token_kind_t vaue (%d).\n", kind);
        terminate();
    }
}

char *token_string(const token_t *token)
{
    token_t tmp = *token;
    expect(&tmp, TOKEN_STRING);
    const char *begin, *end;
    (void)token_parse(*token, &begin, &end);
    begin++;
    end--;
    char *buff = calloc(end - begin + 1, 1);
    memcpy(buff, begin, end - begin);
    return buff;
}

void unexpected_token(const token_t *token, const char *expected)
{
    const char *begin, *end;
    token_kind_t kind = token_parse(*token, &begin, &end);

    print_location(stderr, begin);
    if (kind == TOKEN_EOF)
        fprintf(stderr, "error: Unexpected EOF. Expected %s.\n", expected);
    else
        fprintf(stderr, "error: Unexpected token `%.*s`. Expected %s.\n", (int)(end - begin), begin, expected);
    print_snippet(stderr, begin, end);
}

token_kind_t peek(const token_t *token)
{
    const char *begin, *end;
    return token_parse(*token, &begin, &end);
}

int match(token_t *token, token_kind_t expected_kind)
{
    const char *begin, *end;
    token_kind_t kind = token_parse(*token, &begin, &end);
    if ((expected_kind == ANY_TOKEN && kind != TOKEN_EOF) || kind == expected_kind)
    {
        *token = end;
        return 1;
    }
    else
    {
        return 0;
    }
}

int can_match(const token_t *token, token_kind_t expected_kind)
{
    token_t tmp = *token;
    return match(&tmp, expected_kind);
}

void expect(token_t *token, token_kind_t expected_kind)
{
    if (match(token, expected_kind))
        return;

    unexpected_token(token, token_kind_str(expected_kind));
    terminate();
}

void output(const char *s, FILE *out)
{
    if (out)
        fprintf(out, "%s", s);
}

void output_indented(const char *s, FILE *out, size_t indent)
{
    if (out)
        fprintf(out, "%*s%s", (int)indent * 4, "", s);
}

void output_token(const token_t *token, FILE *out)
{
    const char *begin, *end;
    if (token_parse(*token, &begin, &end))
        fprintf(out, "%.*s", (int)(end - begin), begin);
    else
        fprintf(out, "EOF");
}
