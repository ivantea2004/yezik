#include "helpers.h"
#include "import.h"
#include "error.h"
#include <stdlib.h>

const char *token_kind_str(token_kind_t kind)
{
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

    case TOKEN_NULL:
        return "`null`";
    case TOKEN_UNDEFINED:
        return "`undefined`";
    case TOKEN_TRUE:
        return "`true`";
    case TOKEN_FALSE:
        return "`false`";

    case TOKEN_IF:
        return "`if` keyword";
    case TOKEN_ELSE:
        return "`else` keyword";
    case TOKEN_WHILE:
        return "`while` keyword";
    case TOKEN_BREAK:
        return "`break` keyword";
    case TOKEN_CONTINUE:
        return "`continue` keyword";

    case TOKEN_ASSIGN:
        return "`=`";

    case TOKEN_COMMA:
        return "`,`";
    case TOKEN_COLON:
        return "`:`";
    case TOKEN_SEMI:
        return "`;`";

    default:
        fprintf(stderr, "internal: Unknown token_kind_t vaue (%d).\n", kind);
        terminate();
    }
}

#define ANY_TOKEN ((token_kind_t)(-1))

typedef const char *token_t;

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

void output_indented(const char *s, size_t indent, FILE *out)
{
    if (out)
        fprintf(out, "%*s%s", (int)indent, "", s);
}

void output_token(const token_t *token, FILE *out)
{
    const char *begin, *end;
    if (token_parse(*token, &begin, &end))
        fprintf(out, "%.*s", (int)(end - begin), begin);
    else
        fprintf(out, "EOF");
}
