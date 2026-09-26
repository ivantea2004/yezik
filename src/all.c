#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------- */
/*                                   panic()                                  */
/* -------------------------------------------------------------------------- */

#ifdef panic
#undef panic
#endif

#define panic() (fprintf(stderr, "panic() was called.\n"), abort())

/* -------------------------------------------------------------------------- */
/*                                    Utils                                   */
/* -------------------------------------------------------------------------- */

static char *str_from_range(const char *begin, const char *end)
{
    char *buff = calloc(end - begin + 1, 1);
    strncat(buff, begin, end - begin);
    return buff;
}

static void path_normalize(char *path)
{
    const char *read = path;
    char *write = path;
    for (; *read; write++, read++)
    {
        if (read[0] == '.' && read[1] == '/')
        {
            read++;
            write--;
            continue;
        }
        else if (read[0] == '.' && read[1] == '.' && read[2] == '/')
        {
            read++;
            read++;
            while (*write != '/')
                write--;
            write--;
            while (write >= path && *write != '/')
                write--;
            continue;
        }
        else
        {
            for (; *read && *read != '/'; read++, write++)
                *write = *read;
            *write = *read;
        }
    }
}

typedef struct
{
    char **strs;
    size_t size;
} str_list_t;

static const char *str_list_find(const str_list_t *list, const char *str)
{
    for (size_t i = 0; i < list->size; i++)
        if (strcmp(str, list->strs[i]) == 0)
            return list->strs[i];
    return NULL;
}

static void str_list_append(str_list_t *list, const char *str)
{
    list->strs = realloc(list->strs, sizeof(list->strs) * (list->size + 1));
    list->strs[list->size] = str_from_range(str, str + strlen(str));
    list->size++;
}

/* -------------------------------------------------------------------------- */
/*                                   Globals                                  */
/* -------------------------------------------------------------------------- */

char *current_path;
char *current_text;
FILE *output_file;
str_list_t imported_paths;
str_list_t enums;

static void parse_file(void);
static void import_file(const char *path, const char *begin, const char *end);

/* -------------------------------------------------------------------------- */
/*                              Error formatting                              */
/* -------------------------------------------------------------------------- */

static void find_location(const char *pos, const char **begin, const char **end, size_t *line_number)
{

    const char *i = current_text;
    *line_number = 1;
    *begin = i;

    for (; *i; i++)
        if (*i == '\n')
        {
            if (pos <= i)
            {
                *end = i;
                return;
            }
            else
            {
                *begin = i + 1;
                (*line_number)++;
            }
        }
    *end = i;
}

static void print_location(const char *pos)
{
    const char *line_begin;
    const char *line_end;
    size_t line_number;
    find_location(pos, &line_begin, &line_end, &line_number);
    fprintf(stderr, "%s:%d:%d: ", current_path, (int)line_number, (int)(pos - line_begin) + 1);
}

static void print_snippet(const char *begin, const char *end)
{
    const char *line_begin;
    const char *line_end;
    size_t line_number;
    find_location(begin, &line_begin, &line_end, &line_number);
    if (end <= begin)
        end = begin + 1;
    if (end >= line_end)
        end = line_end;
    size_t offset = fprintf(stderr, " %d ", (int)line_number);
    fprintf(stderr, "| %.*s\n", (int)(line_end - line_begin), line_begin);
    if (end > begin)
    {
        fprintf(stderr, "%*s| ", (int)offset, "");
        for (const char *i = line_begin; i < end; i++)
            fputc(i < begin ? ' ' : i == begin ? '^'
                                               : '~',
                  stderr);
        fputc('\n', stderr);
    }
}

/* -------------------------------------------------------------------------- */
/*                                    Lexer                                   */
/* -------------------------------------------------------------------------- */

#define TOKEN_BUILTINS_X(X)         \
    X(TOKEN_NULL, "null")           \
    X(TOKEN_UNDEFINED, "undefined") \
    X(TOKEN_TRUE, "true")           \
    X(TOKEN_FALSE, "false")         \
    X(TOKEN_UNDER, "_")             \
    X(TOKEN_TYPE, "type")           \
    X(TOKEN_BOOL, "bool")

#define TOKEN_KEYWORDS_X(X)       \
    X(TOKEN_IF, "if")             \
    X(TOKEN_ELSE, "else")         \
    X(TOKEN_WHILE, "while")       \
    X(TOKEN_BREAK, "break")       \
    X(TOKEN_CONTINUE, "continue") \
    X(TOKEN_RETURN, "return")     \
                                  \
    X(TOKEN_IMPORT, "import")     \
    X(TOKEN_CONST, "const")       \
    X(TOKEN_LET, "let")           \
    X(TOKEN_RECORD, "record")     \
    X(TOKEN_ENUM, "enum")

#define TOKEN_KEYWORD_OPERATORS_X(X) \
    X(TOKEN_NOT, "not")              \
    X(TOKEN_AND, "and")              \
    X(TOKEN_OR, "or")                \
    X(TOKEN_CAST, "cast")

#define TOKEN_SYMBOLS_X(X)      \
                                \
    X(TOKEN_COMMA, ",")         \
    X(TOKEN_COLON, ":")         \
    X(TOKEN_SEMI, ";")          \
                                \
    X(TOKEN_LPAR, "(")          \
    X(TOKEN_RPAR, ")")          \
    X(TOKEN_LCUR, "{")          \
    X(TOKEN_RCUR, "}")          \
    X(TOKEN_LBR, "[")           \
    X(TOKEN_RBR, "]")           \
                                \
    X(TOKEN_EQ, "==")           \
    X(TOKEN_NE, "<>")           \
    X(TOKEN_LE, "<=")           \
    X(TOKEN_LT, "<")            \
    X(TOKEN_GE, ">=")           \
    X(TOKEN_GT, ">")            \
                                \
    X(TOKEN_ARROW, "=>")        \
                                \
    X(TOKEN_ASSIGN, "=")        \
                                \
    X(TOKEN_PLUS_ASSIGN, "+=")  \
    X(TOKEN_MINUS_ASSIGN, "-=") \
    X(TOKEN_MULT_ASSIGN, "*=")  \
    X(TOKEN_DIV_ASSIGN, "/=")   \
                                \
    X(TOKEN_PLUS, "+")          \
    X(TOKEN_MINUS, "-")         \
    X(TOKEN_MULT, "*")          \
    X(TOKEN_DIV, "/")           \
    X(TOKEN_MOD, "%")           \
                                \
    X(TOKEN_REF, "&")           \
    X(TOKEN_DEREF, "^")         \
                                \
    X(TOKEN_DOT, ".")

#define TOKEN(x, ...) x,

typedef enum
{
    TOKEN_EOF,
    TOKEN_ID,
    TOKEN_INT,
    TOKEN_STR,
    TOKEN_BUILTINS_X(TOKEN)
    TOKEN_KEYWORDS_X(TOKEN) TOKEN_KEYWORD_OPERATORS_X(TOKEN) TOKEN_SYMBOLS_X(TOKEN)
} token_kind_t;

#undef TOKEN

static const char *token_kind_str(token_kind_t kind)
{

#define KEYWORD_CASE(kind, str) \
    case kind:                  \
        return "'" str "' keyword";

#define SYMBOL_CASE(kind, str) \
    case kind:                 \
        return "'" str "'";

    switch (kind)
    {
    case TOKEN_EOF:
        return "EOF";
    case TOKEN_ID:
        return "identifier";
    case TOKEN_INT:
        return "integer literal";
    case TOKEN_STR:
        return "string literal";
        TOKEN_BUILTINS_X(SYMBOL_CASE);
        TOKEN_KEYWORDS_X(KEYWORD_CASE);
        TOKEN_KEYWORD_OPERATORS_X(KEYWORD_CASE);
        TOKEN_SYMBOLS_X(SYMBOL_CASE);
    default:
        fprintf(stderr, "internal: Unknown token_kind_t value (%d).\n", kind);
        panic();
    }
#undef SYMBOL_CASE
#undef KEYWORD_CASE
}

static int is_space(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static int is_number(char c)
{
    return '0' <= c && c <= '9';
}

static int is_allowed_in_id(char c)
{
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_' || is_number(c);
}

static const char *lexer_skip_space(const char *pos)
{
    while (*pos && is_space(*pos))
        pos++;
    return pos;
}

static const char *lexer_get_token(const char *pos, const char **begin, const char **end, token_kind_t *kind)
{

#define KEYWORD_MATCH(k, str)                                                           \
    if (*end - *begin == sizeof(str) - 1 && strncmp(str, *begin, sizeof(str) - 1) == 0) \
    {                                                                                   \
        *kind = k;                                                                      \
        return lexer_skip_space(pos);                                                   \
    }

#define SYMBOL_MATCH(k, str)         \
    do                               \
    {                                \
        const char *i = str;         \
        const char *q = pos;         \
        while (*i && *q && *i == *q) \
            i++, q++;                \
        if (*i)                      \
            break;                   \
        *end = q;                    \
        *kind = k;                   \
        return lexer_skip_space(q);  \
    } while (0);

    const char *expected;
    pos = lexer_skip_space(pos);

    *begin = pos;

    if (!*pos)
    {
        *end = pos;
        *kind = TOKEN_EOF;
        return pos;
    }
    else if (is_number(*pos))
    {
        while (*pos && is_number(*pos))
            pos++;
        *end = pos;
        *kind = TOKEN_INT;
        return lexer_skip_space(pos);
    }
    else if (is_allowed_in_id(*pos))
    {
        while (*pos && is_allowed_in_id(*pos))
            pos++;
        *end = pos;

        TOKEN_BUILTINS_X(KEYWORD_MATCH);
        TOKEN_KEYWORDS_X(KEYWORD_MATCH);
        TOKEN_KEYWORD_OPERATORS_X(KEYWORD_MATCH)

        *kind = TOKEN_ID;
        return lexer_skip_space(pos);
    }
    else if (*pos == '"')
    {
        *begin = pos;
        pos++;
        while (*pos && *pos != '"')
            pos++;
        if (*pos == '"')
        {
            pos++;
            *end = pos;
            *kind = TOKEN_STR;
            return lexer_skip_space(pos);
        }
        else
        {
            expected = "matching '\"'";
            goto unexpected_eof;
        }
    }
    else if (*pos == '\'')
    {
        *begin = pos;
        pos++;
        while (*pos && *pos != '\'')
            pos++;
        if (*pos == '\'')
        {
            pos++;
            *end = pos;
            *kind = TOKEN_STR;
            return lexer_skip_space(pos);
        }
        else
        {
            expected = "matching '\''";
            goto unexpected_eof;
        }
    }
    else
    {
        TOKEN_SYMBOLS_X(SYMBOL_MATCH);

        print_location(pos);
        fprintf(stderr, "error: Unexpected char `%c`.\n", *pos);
        print_snippet(pos, pos + 1);
        panic();
    }

unexpected_eof:
    print_location(pos);
    fprintf(stderr, "error: Unexpected EOF. Expected %s \n", expected);
    print_snippet(pos - 1, pos);
    panic();

#undef SYMBOL_MATCH
#undef KEYWORD_MATCH
}

/* -------------------------------------------------------------------------- */
/*                                   Codegen                                  */
/* -------------------------------------------------------------------------- */

static void output(const char *str, int skip)
{
    if (!skip)
        fprintf(output_file, "%s", str);
}

static void output_indent(size_t indent, int skip)
{
    while (indent)
    {
        output("    ", skip);
        indent--;
    }
}

static void output_token_id(const char *pos, int skip)
{
    const char *begin;
    const char *end;
    token_kind_t kind;
    (void)lexer_get_token(pos, &begin, &end, &kind);
    if (kind != TOKEN_ID)
        panic();
    if (!skip)
        fprintf(output_file, "%.*s", (int)(end - begin), begin);
}

static void output_token_int(const char *pos, int skip)
{
    const char *begin;
    const char *end;
    token_kind_t kind;
    (void)lexer_get_token(pos, &begin, &end, &kind);
    if (kind != TOKEN_INT)
        panic();
    if (!skip)
        fprintf(output_file, "%.*s", (int)(end - begin), begin);
}

static void output_token_str(const char *pos, int skip)
{
    const char *begin;
    const char *end;
    token_kind_t kind;
    (void)lexer_get_token(pos, &begin, &end, &kind);
    if (kind != TOKEN_STR)
        panic();
    if (!skip)
        fprintf(output_file, "\"%.*s\"", (int)(end - begin - 2), begin + 1);
}

static void output_token_char(const char *pos, int skip)
{
    const char *begin;
    const char *end;
    token_kind_t kind;
    (void)lexer_get_token(pos, &begin, &end, &kind);
    if (kind != TOKEN_STR)
        panic();
    if (!skip)
        fprintf(output_file, "'%.*s'", (int)(end - begin - 2), begin + 1);
}

/* -------------------------------------------------------------------------- */
/*                                   Parser                                   */
/* -------------------------------------------------------------------------- */

static token_kind_t token_peek(const char *pos)
{
    const char *begin;
    const char *end;
    token_kind_t kind;
    (void)lexer_get_token(pos, &begin, &end, &kind);
    return kind;
}

static const char *token_step(const char *pos)
{
    const char *begin;
    const char *end;
    token_kind_t kind;
    return lexer_get_token(pos, &begin, &end, &kind);
}

static void token_unexpected(const char *pos, const char *expected)
{
    const char *begin = NULL;
    const char *end = NULL;
    token_kind_t kind;
    (void)lexer_get_token(pos, &begin, &end, &kind);

    print_location(begin);
    if (kind == TOKEN_EOF)
        fprintf(stderr, "error: Unexpected EOF. Expected %s.\n", expected);
    else
        fprintf(stderr, "error: Unexpected token '%.*s'. Expected %s.\n", (int)(end - begin), begin, expected);
    print_snippet(begin, end);
}

static const char *token_expect(const char *pos, token_kind_t expected_kind)
{
    token_kind_t kind = token_peek(pos);
    if (kind == expected_kind)
    {
        return token_step(pos);
    }
    else
    {
        token_unexpected(pos, token_kind_str(expected_kind));
        panic();
    }
}

static token_kind_t token_peek_ex(const char *pos, size_t depth)
{
    token_kind_t cur = token_peek(pos);
    if (depth == 0)
        return cur;
    if (cur == TOKEN_EOF)
        return TOKEN_EOF;
    return token_peek_ex(token_step(pos), depth - 1);
}

static const char *parse_type(const char *pos, int skip)
{
    if (token_peek_ex(pos, 0) == TOKEN_REF &&
        token_peek_ex(pos, 1) == TOKEN_LBR)
    {
        pos = token_expect(pos, TOKEN_REF);
        pos = token_expect(pos, TOKEN_LBR);
        pos = token_expect(pos, TOKEN_UNDER);
        pos = token_expect(pos, TOKEN_RBR);
        pos = parse_type(pos, skip);
        output("*", skip);
        return pos;
    }

    if (token_peek(pos) == TOKEN_REF)
    {
        pos = token_expect(pos, TOKEN_REF);
        pos = parse_type(pos, skip);
        output("*", skip);
        return pos;
    }

    if (token_peek(pos) == TOKEN_BOOL)
    {
        output("int", skip);
        return token_step(pos);
    }

    token_expect(pos, TOKEN_ID);
    output_token_id(pos, skip);
    return token_step(pos);
}

static const char *parse_expr(const char *pos, int skip);

static const char *parse_unary_expr(const char *pos, int skip)
{
    if (token_peek_ex(pos, 0) == TOKEN_CAST &&
        token_peek_ex(pos, 1) == TOKEN_LPAR &&
        token_peek_ex(pos, 2) == TOKEN_ID &&
        token_peek_ex(pos, 3) == TOKEN_RPAR &&
        token_peek_ex(pos, 4) == TOKEN_STR)
    {
        pos = token_expect(pos, TOKEN_CAST);
        pos = token_expect(pos, TOKEN_LPAR);
        pos = token_expect(pos, TOKEN_ID);
        pos = token_expect(pos, TOKEN_RPAR);
        output_token_char(pos, skip);
        return token_expect(pos, TOKEN_STR);
    }

    if (token_peek_ex(pos, 0) == TOKEN_ID &&
        token_peek_ex(pos, 1) == TOKEN_DOT &&
        token_peek_ex(pos, 2) == TOKEN_ID)
    {
        const char *begin;
        const char *end;
        token_kind_t kind;
        lexer_get_token(pos, &begin, &end, &kind);
        char *name = str_from_range(begin, end);
        if (str_list_find(&enums, name))
        {
            output_token_id(pos, skip);
            output("_", skip);
            pos = token_expect(pos, TOKEN_ID);
            pos = token_expect(pos, TOKEN_DOT);
            output_token_id(pos, skip);
            pos = token_expect(pos, TOKEN_ID);
            return pos;
        }
        free(name);
    }

    while (1)
    {
        if (token_peek(pos) == TOKEN_PLUS)
        {
            output("+", skip);
            pos = token_step(pos);
        }
        else if (token_peek(pos) == TOKEN_MINUS)
        {
            output("-", skip);
            pos = token_step(pos);
        }
        else if (token_peek(pos) == TOKEN_NOT)
        {
            output("!", skip);
            pos = token_step(pos);
        }
        else if (token_peek(pos) == TOKEN_REF)
        {
            output("&", skip);
            pos = token_step(pos);
        }
        else if (token_peek(pos) == TOKEN_CAST)
        {
            pos = token_expect(pos, TOKEN_CAST);
            pos = token_expect(pos, TOKEN_LPAR);
            output("(", skip);
            pos = parse_type(pos, skip);
            pos = token_expect(pos, TOKEN_RPAR);
            output(")", skip);
        }
        else
        {
            break;
        }
    }

    if (token_peek(pos) == TOKEN_INT)
    {
        output_token_int(pos, skip);
        pos = token_step(pos);
    }
    else if (token_peek(pos) == TOKEN_STR)
    {
        output_token_str(pos, skip);
        pos = token_step(pos);
    }
    else if (token_peek(pos) == TOKEN_ID)
    {
        output_token_id(pos, skip);
        pos = token_step(pos);
    }
    else if (token_peek(pos) == TOKEN_LPAR)
    {
        output("(", skip);
        pos = token_expect(pos, TOKEN_LPAR);
        pos = parse_expr(pos, skip);
        pos = token_expect(pos, TOKEN_RPAR);
        output(")", skip);
    }
    else if (token_peek(pos) == TOKEN_NULL)
    {
        output("NULL", skip);
        pos = token_step(pos);
    }
    else if (token_peek(pos) == TOKEN_TRUE)
    {
        output("1", skip);
        pos = token_step(pos);
    }
    else if (token_peek(pos) == TOKEN_FALSE)
    {
        output("0", skip);
        pos = token_step(pos);
    }
    else
    {
        token_unexpected(pos, "expression");
        panic();
    }

    while (1)
    {
        if (token_peek(pos) == TOKEN_DEREF)
        {
            output("[0]", skip);
            pos = token_step(pos);
        }
        else if (token_peek(pos) == TOKEN_DOT)
        {
            pos = token_expect(pos, TOKEN_DOT);
            output(".", skip);
            output_token_id(pos, skip);
            pos = token_expect(pos, TOKEN_ID);
        }
        else if (token_peek(pos) == TOKEN_LBR)
        {
            output("[", skip);
            pos = token_expect(pos, TOKEN_LBR);
            pos = parse_expr(pos, skip);
            pos = token_expect(pos, TOKEN_RBR);
            output("]", skip);
        }
        else if (token_peek(pos) == TOKEN_LPAR)
        {
            pos = token_expect(pos, TOKEN_LPAR);
            output("(", skip);

            if (token_peek(pos) != TOKEN_RPAR)
                while (1)
                {
                    pos = parse_expr(pos, skip);
                    if (token_peek(pos) == TOKEN_RPAR)
                        break;
                    else
                    {
                        output(", ", skip);
                        pos = token_expect(pos, TOKEN_COMMA);
                    }
                }

            pos = token_expect(pos, TOKEN_RPAR);
            output(")", skip);
        }
        else
        {
            break;
        }
    }

    return pos;
}

static const char *parse_expr(const char *pos, int skip)
{
    pos = parse_unary_expr(pos, skip);
    if (token_peek(pos) == TOKEN_PLUS)
    {
        output(" + ", skip);
        pos = token_step(pos);
        return parse_expr(pos, skip);
    }
    else if (token_peek(pos) == TOKEN_MINUS)
    {
        output(" - ", skip);
        pos = token_step(pos);
        return parse_expr(pos, skip);
    }
    else if (token_peek(pos) == TOKEN_MULT)
    {
        output(" * ", skip);
        pos = token_step(pos);
        return parse_expr(pos, skip);
    }
    else if (token_peek(pos) == TOKEN_DIV)
    {
        output(" / ", skip);
        pos = token_step(pos);
        return parse_expr(pos, skip);
    }
    else if (token_peek(pos) == TOKEN_MOD)
    {
        output(" % ", skip);
        pos = token_step(pos);
        return parse_expr(pos, skip);
    }
    else if (token_peek(pos) == TOKEN_AND)
    {
        output(" && ", skip);
        pos = token_step(pos);
        return parse_expr(pos, skip);
    }
    else if (token_peek(pos) == TOKEN_OR)
    {
        output(" OR ", skip);
        pos = token_step(pos);
        return parse_expr(pos, skip);
    }
    else if (token_peek(pos) == TOKEN_LT)
    {
        output(" < ", skip);
        pos = token_step(pos);
        return parse_expr(pos, skip);
    }
    else if (token_peek(pos) == TOKEN_LE)
    {
        output(" <= ", skip);
        pos = token_step(pos);
        return parse_expr(pos, skip);
    }
    else if (token_peek(pos) == TOKEN_GT)
    {
        output(" > ", skip);
        pos = token_step(pos);
        return parse_expr(pos, skip);
    }
    else if (token_peek(pos) == TOKEN_GE)
    {
        output(" >= ", skip);
        pos = token_step(pos);
        return parse_expr(pos, skip);
    }
    else if (token_peek(pos) == TOKEN_EQ)
    {
        output(" == ", skip);
        pos = token_step(pos);
        return parse_expr(pos, skip);
    }
    else if (token_peek(pos) == TOKEN_NE)
    {
        output(" != ", skip);
        pos = token_step(pos);
        return parse_expr(pos, skip);
    }
    else if (token_peek(pos) == TOKEN_ASSIGN)
    {
        output(" = ", skip);
        pos = token_step(pos);
        return parse_expr(pos, skip);
    }
    else if (token_peek(pos) == TOKEN_PLUS_ASSIGN)
    {
        output(" += ", skip);
        pos = token_step(pos);
        return parse_expr(pos, skip);
    }
    else if (token_peek(pos) == TOKEN_MINUS_ASSIGN)
    {
        output(" -= ", skip);
        pos = token_step(pos);
        return parse_expr(pos, skip);
    }
    else if (token_peek(pos) == TOKEN_MULT_ASSIGN)
    {
        output(" *= ", skip);
        pos = token_step(pos);
        return parse_expr(pos, skip);
    }
    else if (token_peek(pos) == TOKEN_DIV_ASSIGN)
    {
        output(" /= ", skip);
        pos = token_step(pos);
        return parse_expr(pos, skip);
    }
    else
    {
        return pos;
    }
}

static const char *parse_stmt(const char *pos, int skip, size_t indent)
{
    if (token_peek(pos) == TOKEN_LCUR)
    {
        pos = token_step(pos);
        output_indent(indent, skip);
        output("{\n", skip);
        while (1)
        {
            if (token_peek(pos) == TOKEN_RCUR)
            {
                output_indent(indent, skip);
                output("}\n", skip);
                return token_step(pos);
            }
            pos = parse_stmt(pos, skip, indent + 1);
        }
    }

    if (token_peek(pos) == TOKEN_LET)
    {
        pos = token_step(pos);
        output_indent(indent, skip);
        const char *name = pos;
        pos = token_expect(pos, TOKEN_ID);
        pos = token_expect(pos, TOKEN_COLON);
        pos = parse_type(pos, skip);
        output(" ", skip);
        output_token_id(name, skip);
        pos = token_expect(pos, TOKEN_ASSIGN);
        if (token_peek(pos) != TOKEN_UNDEFINED)
        {
            output(" = ", skip);
            pos = parse_expr(pos, skip);
        }
        else
            pos = token_step(pos);
        output(";\n", skip);
        return token_expect(pos, TOKEN_SEMI);
    }

    if (token_peek(pos) == TOKEN_BREAK)
    {
        pos = token_step(pos);
        output_indent(indent, skip);
        output("break ", skip);
        if (token_peek(pos) != TOKEN_SEMI)
            pos = parse_expr(pos, skip);
        output(";\n", skip);
        return token_expect(pos, TOKEN_SEMI);
    }

    if (token_peek(pos) == TOKEN_CONTINUE)
    {
        output_indent(indent, skip);
        output("continue;\n", skip);
        return token_expect(pos, TOKEN_SEMI);
    }

    if (token_peek(pos) == TOKEN_RETURN)
    {
        pos = token_step(pos);
        output_indent(indent, skip);
        output("return ", skip);
        if (token_peek(pos) != TOKEN_SEMI)
            pos = parse_expr(pos, skip);
        output(";\n", skip);
        return token_expect(pos, TOKEN_SEMI);
    }

    if (token_peek(pos) == TOKEN_WHILE)
    {
        pos = token_step(pos);
        output_indent(indent, skip);
        output("while (", skip);
        pos = parse_expr(pos, skip);
        output(")\n", skip);
        token_expect(pos, TOKEN_LCUR);
        return parse_stmt(pos, skip, indent);
    }

    if (token_peek(pos) == TOKEN_IF)
    {
        pos = token_step(pos);
        output_indent(indent, skip);
        output("if (", skip);
        pos = parse_expr(pos, skip);
        output(")\n", skip);
        token_expect(pos, TOKEN_LCUR);
        pos = parse_stmt(pos, skip, indent);
        if (token_peek(pos) == TOKEN_ELSE)
        {
            pos = token_step(pos);
            output_indent(indent, skip);
            output("else\n", skip);
            if (token_peek(pos) != TOKEN_IF)
                token_expect(pos, TOKEN_LCUR);
            return parse_stmt(pos, skip, indent);
        }
        return pos;
    }

    if (token_peek(pos) == TOKEN_UNDER)
    {
        pos = token_expect(pos, TOKEN_UNDER);
        pos = token_expect(pos, TOKEN_ASSIGN);
        output_indent(indent, skip);
        output("(void)", skip);
        pos = parse_expr(pos, skip);
        output(";\n", skip);
        return token_expect(pos, TOKEN_SEMI);
    }

    output_indent(indent, skip);
    pos = parse_expr(pos, skip);
    output(";\n", skip);
    return token_expect(pos, TOKEN_SEMI);
}

static const char *parse_func_sig_args(const char *pos, int skip)
{
    pos = token_expect(pos, TOKEN_LPAR);
    if (token_peek(pos) == TOKEN_RPAR)
    {
        output("(void)", skip);
        return token_step(pos);
    }
    output("(", skip);
    while (1)
    {
        const char *name = pos;
        pos = token_expect(pos, TOKEN_ID);
        pos = token_expect(pos, TOKEN_COLON);
        pos = parse_type(pos, skip);
        output(" ", skip);
        output_token_id(name, skip);

        if (token_peek(pos) == TOKEN_RPAR)
            break;
        else
        {
            pos = token_expect(pos, TOKEN_COMMA);
            output(", ", skip);
        }
    }
    output(")", skip);
    return token_expect(pos, TOKEN_RPAR);
}

static const char *parse_global(const char *pos, int def)
{
    if (token_peek(pos) == TOKEN_IMPORT)
    {
        pos = token_step(pos);
        const char *begin;
        const char *end;
        token_kind_t kind;
        token_expect(pos, TOKEN_STR);
        lexer_get_token(pos, &begin, &end, &kind);

        if (!def)
        {
            char *path = str_from_range(begin + 1, end - 1);
            import_file(path, begin, end);
            free(path);
        }

        pos = token_step(pos);
        return token_expect(pos, TOKEN_SEMI);
    }

    if (token_peek_ex(pos, 0) == TOKEN_CONST &&
        token_peek_ex(pos, 1) == TOKEN_ID &&
        token_peek_ex(pos, 2) == TOKEN_ASSIGN &&
        token_peek_ex(pos, 3) == TOKEN_RECORD)
    {
        pos = token_expect(pos, TOKEN_CONST);
        const char *name = pos;
        pos = token_expect(pos, TOKEN_ID);
        pos = token_expect(pos, TOKEN_ASSIGN);
        pos = token_expect(pos, TOKEN_RECORD);
        pos = token_expect(pos, TOKEN_LCUR);

        output("typedef struct ", def);
        output_token_id(name, def);
        output(" ", def);
        output_token_id(name, def);
        output(";\n", def);

        output("struct ", !def);
        output_token_id(name, !def);
        output("\n{\n", !def);

        while (1)
        {
            if (token_peek(pos) == TOKEN_RCUR)
                break;
            const char *member = pos;
            pos = token_expect(pos, TOKEN_ID);
            pos = token_expect(pos, TOKEN_COLON);
            output_indent(1, !def);
            pos = parse_type(pos, !def);
            output(" ", !def);
            output_token_id(member, !def);
            output(";\n", !def);
            pos = token_expect(pos, TOKEN_SEMI);
        }
        pos = token_expect(pos, TOKEN_RCUR);
        output("};\n", !def);
        return token_expect(pos, TOKEN_SEMI);
    }

    if (token_peek_ex(pos, 0) == TOKEN_CONST &&
        token_peek_ex(pos, 1) == TOKEN_ID &&
        token_peek_ex(pos, 2) == TOKEN_ASSIGN &&
        token_peek_ex(pos, 3) == TOKEN_ENUM)
    {
        pos = token_expect(pos, TOKEN_CONST);
        const char *name = pos;
        pos = token_expect(pos, TOKEN_ID);
        pos = token_expect(pos, TOKEN_ASSIGN);
        pos = token_expect(pos, TOKEN_ENUM);
        pos = token_expect(pos, TOKEN_LCUR);

        output("enum ", def);
        output_token_id(name, def);
        output("\n{\n", def);

        while (1)
        {
            if (token_peek(pos) == TOKEN_RCUR)
                break;
            output_indent(1, def);
            output_token_id(name, def);
            output("_", def);
            output_token_id(pos, def);
            pos = token_expect(pos, TOKEN_ID);
            pos = token_expect(pos, TOKEN_SEMI);
            if (token_peek(pos) == TOKEN_RCUR)
                output("\n", def);
            else
                output(",\n", def);
        }
        pos = token_expect(pos, TOKEN_RCUR);
        output("};\n", def);

        output("typedef enum ", def);
        output_token_id(name, def);
        output(" ", def);
        output_token_id(name, def);
        output(";\n", def);
        {
            const char *begin;
            const char *end;
            token_kind_t kind;
            lexer_get_token(name, &begin, &end, &kind);
            char *name = str_from_range(begin, end);
            str_list_append(&enums, name);
            free(name);
        }
        return token_expect(pos, TOKEN_SEMI);
    }

    if (token_peek_ex(pos, 0) == TOKEN_CONST &&
        token_peek_ex(pos, 1) == TOKEN_ID &&
        token_peek_ex(pos, 2) == TOKEN_COLON &&
        token_peek_ex(pos, 3) == TOKEN_LPAR)
    {
        pos = token_expect(pos, TOKEN_CONST);
        const char *name = pos;
        pos = token_expect(pos, TOKEN_ID);
        pos = token_expect(pos, TOKEN_COLON);
        pos = parse_func_sig_args(pos, 1);
        pos = token_expect(pos, TOKEN_COLON);
        pos = parse_type(pos, !def);
        output(" ", !def);
        output_token_id(name, !def);
        pos = token_expect(name, TOKEN_ID);
        pos = token_expect(pos, TOKEN_COLON);
        pos = parse_func_sig_args(pos, !def);
        pos = token_expect(pos, TOKEN_COLON);
        pos = parse_type(pos, 1);
        pos = token_expect(pos, TOKEN_ASSIGN);
        pos = token_expect(pos, TOKEN_UNDEFINED);
        output(";\n", !def);
        return token_expect(pos, TOKEN_SEMI);
    }

    if (token_peek_ex(pos, 0) == TOKEN_CONST &&
        token_peek_ex(pos, 1) == TOKEN_ID &&
        token_peek_ex(pos, 2) == TOKEN_ASSIGN &&
        token_peek_ex(pos, 3) == TOKEN_LPAR)
    {
        pos = token_expect(pos, TOKEN_CONST);
        const char *name = pos;
        pos = token_expect(pos, TOKEN_ID);
        pos = token_expect(pos, TOKEN_ASSIGN);
        pos = parse_func_sig_args(pos, 1);
        pos = token_expect(pos, TOKEN_COLON);
        pos = parse_type(pos, 0);
        output(" ", 0);
        output_token_id(name, 0);
        pos = token_expect(name, TOKEN_ID);
        pos = token_expect(pos, TOKEN_ASSIGN);
        pos = parse_func_sig_args(pos, 0);
        pos = token_expect(pos, TOKEN_COLON);
        pos = parse_type(pos, 1);
        pos = token_expect(pos, TOKEN_ARROW);
        token_expect(pos, TOKEN_LCUR);
        if (!def)
            output(";\n", 0);
        else
            output("\n", 0);
        pos = parse_stmt(pos, !def, 0);
        return token_expect(pos, TOKEN_SEMI);
    }

    if (token_peek_ex(pos, 0) == TOKEN_CONST &&
        token_peek_ex(pos, 1) == TOKEN_ID &&
        token_peek_ex(pos, 2) == TOKEN_ASSIGN)
    {
        output("#define ", def);
        pos = token_step(pos);
        output_token_id(pos, def);
        pos = token_step(pos);
        pos = token_step(pos);
        output(" (", def);
        pos = parse_expr(pos, def);
        output(")\n", def);
        return token_expect(pos, TOKEN_SEMI);
    }

    if (token_peek(pos) == TOKEN_LET)
    {
        pos = token_expect(pos, TOKEN_LET);
        const char *name = pos;
        pos = token_expect(pos, TOKEN_ID);
        pos = token_expect(pos, TOKEN_COLON);
        pos = parse_type(pos, def);
        output(" ", def);
        output_token_id(name, def);
        pos = token_expect(pos, TOKEN_ASSIGN);
        output(" = ", def);
        pos = parse_expr(pos, def);
        output(";\n", def);
        return token_expect(pos, TOKEN_SEMI);
    }

    if (token_peek_ex(pos, 0) == TOKEN_CONST &&
        token_peek_ex(pos, 1) == TOKEN_ID &&
        token_peek_ex(pos, 2) == TOKEN_COLON &&
        token_peek_ex(pos, 3) == TOKEN_TYPE &&
        token_peek_ex(pos, 4) == TOKEN_ASSIGN &&
        token_peek_ex(pos, 5) == TOKEN_UNDEFINED)
    {
        pos = token_expect(pos, TOKEN_CONST);
        pos = token_expect(pos, TOKEN_ID);
        pos = token_expect(pos, TOKEN_COLON);
        pos = token_expect(pos, TOKEN_TYPE);
        pos = token_expect(pos, TOKEN_ASSIGN);
        pos = token_expect(pos, TOKEN_UNDEFINED);
        return token_expect(pos, TOKEN_SEMI);
    }

    if (token_peek_ex(pos, 0) == TOKEN_CONST &&
        token_peek_ex(pos, 1) == TOKEN_ID &&
        token_peek_ex(pos, 2) == TOKEN_COLON &&
        token_peek_ex(pos, 3) == TOKEN_TYPE)
    {
        pos = token_expect(pos, TOKEN_CONST);
        const char *name = pos;
        pos = token_expect(pos, TOKEN_ID);
        pos = token_expect(pos, TOKEN_COLON);
        pos = token_expect(pos, TOKEN_TYPE);
        pos = token_expect(pos, TOKEN_ASSIGN);
        output("typedef ", def);
        pos = parse_type(pos, def);
        output(" ", def);
        output_token_id(name, def);
        output(";\n", def);
        return token_expect(pos, TOKEN_SEMI);
    }

    token_unexpected(pos, "global");
    panic();
}

void parse_file(void)
{
    const char *pos = current_text;
    while (1)
    {
        pos = lexer_skip_space(pos);
        if (token_peek(pos) == TOKEN_EOF)
            break;
        pos = parse_global(pos, 0);
    }
    pos = current_text;
    while (1)
    {
        pos = lexer_skip_space(pos);
        if (token_peek(pos) == TOKEN_EOF)
            break;
        pos = parse_global(pos, 1);
    }
}

/* -------------------------------------------------------------------------- */
/*                                    Main                                    */
/* -------------------------------------------------------------------------- */

void import_file(const char *path, const char *begin, const char *end)
{
    if (current_path)
    {
        size_t buff_len = strlen(current_path) + strlen("/../") + strlen(path) + 1;
        char *buff = calloc(buff_len, 1);
        strcat(buff, current_path);
        strcat(buff, "/../");
        strcat(buff, path);
        path_normalize(buff);
        path = buff;
    }
    else
    {
        path = str_from_range(path, path + strlen(path));
    }
    if (str_list_find(&imported_paths, path))
    {
        return;
    }

    char *tmp_path = current_path;
    char *tmp_text = current_text;

    errno = 0;
    FILE *file = fopen(path, "rb");
    if (!file)
    {
        if (begin)
            print_location(begin);
        fprintf(stderr, "error: %s: %s.\n", path, strerror(errno));
        if (begin)
            print_snippet(begin, end);
        panic();
    }
    fseek(file, 0, SEEK_END);
    size_t input_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    current_text = calloc(input_size + 1, 1);
    if (fread(current_text, 1, input_size, file) < input_size)
    {
        fprintf(stderr, "error: %s: %s.\n", path, strerror(errno));
        exit(1);
    }
    fclose(file);

    str_list_append(&imported_paths, path);
    current_path = (char *)path;
    parse_file();

    current_path = tmp_path;
    current_text = tmp_text;
}

int main(int argc, char **argv)
{

    if (argc != 3)
    {
        fprintf(stderr, "error: Expected arguments [in].yezik [out].c\n");
        exit(1);
    }

    errno = 0;
    output_file = fopen(argv[2], "w");
    if (!output_file)
    {
        fprintf(stderr, "error: %s: %s\n", argv[2], strerror(errno));
        exit(1);
    }

    import_file(argv[1], NULL, NULL);
    fclose(output_file);
    return 0;
}
