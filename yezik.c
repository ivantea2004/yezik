#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
    Common global stuff
*/
static const char *input_path;
static const char *input_text;
static FILE *output_file;

/*
    Error formatting
*/
static void find_line(const char *place, const char **begin, const char **end, size_t *line_number);
static void print_location(FILE *stream, const char *place);
static void print_snippet(FILE *stream, const char *begin, const char *end);

/*
    Token definition
*/

typedef const char *token_t;

typedef enum
{
    TOKEN_KIND_EOF = 1,
    TOKEN_KIND_ID,
    TOKEN_KIND_LITERAL,
    MAX_TOKEN_KIND
} token_kind_t;

#define TOKEN_EOF (char *)TOKEN_KIND_EOF
#define TOKEN_ID (char *)TOKEN_KIND_ID
#define TOKEN_LITERAL (char *)TOKEN_KIND_LITERAL

static const char *token_kind_str(token_kind_t kind);
static int token_is_token_kind(const char *what);
static token_kind_t token_to_token_kind(const char *what);

/*
    Token parsing
*/
static int is_space(char c);
static int is_digit(char c);
static int is_id_char(char c);
static token_kind_t parse_token(const token_t *token, const char **begin, const char **end);

/*
    Token matching
*/
static int token_can_match(const token_t *token, const char *what);
static int token_match(token_t *token, const char *what);
static void token_expect(token_t *token, const char *what);
static void token_unexpected(const token_t *token, const char *expected);

/*
    Emit functions
*/
static void emit(const char *str, int skip);
static void emit_indented(const char *str, int skip, size_t indent);
static void emit_token(token_t *token, int skip);

/*
    Parsing functions
*/
static void parse_type(token_t *token, int skip);
static void parse_var_decl(token_t *token, int skip);
static void parse_stmt(token_t *token, int skip, size_t indent);
static void parse_stmt_block(token_t *token, int skip, size_t indent);
static void parse_expr(token_t *token, int skip);

/*
    Main
*/
int main(int argc, char **argv)
{

    if (argc < 3)
    {
        fprintf(stderr, "Too few arguments.\n");
        exit(1);
    }

    input_path = argv[1];
    const char *output_path = argv[2];

    {
        FILE *input_file = fopen(input_path, "rb");
        if (!input_file)
        {
            perror(input_path);
            exit(1);
        }
        fseek(input_file, 0, SEEK_END);
        size_t input_size = ftell(input_file);
        fseek(input_file, 0, SEEK_SET);
        input_text = calloc(input_size + 1, 1);
        if (fread((char *)input_text, 1, input_size, input_file) < input_size)
        {
            perror(input_path);
            exit(1);
        }
        fclose(input_file);
    }

    output_file = fopen(output_path, "w");

    if (!output_file)
    {
        perror(output_path);
        exit(1);
    }

    {
        token_t i = input_text;
        parse_stmt(&i, 0, 0);
    }

    fclose(output_file);
    free((char *)input_text);

    return 0;
}

/*
    Error formatting
*/
void find_line(const char *place, const char **begin, const char **end, size_t *line_number)
{
    const char *i = input_text;
    *line_number = 1;
    *begin = input_text;

    for (; *i; i++)
        if (*i == '\n')
        {
            if (place <= i)
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

void print_location(FILE *stream, const char *place)
{
    const char *line_begin;
    const char *line_end;
    size_t line_number;
    find_line(place, &line_begin, &line_end, &line_number);
    fprintf(stream, "%s:%d:%d: ", input_path, (int)line_number, (int)(place - line_begin) + 1);
}

void print_snippet(FILE *stream, const char *begin, const char *end)
{
    const char *line_begin;
    const char *line_end;
    size_t line_number;
    find_line(begin, &line_begin, &line_end, &line_number);
    if (end >= line_end)
        end = line_end;
    size_t offset = fprintf(stream, " %d ", (int)line_number);
    fprintf(stream, "| %.*s\n", (int)(line_end - line_begin), line_begin);
    if (end > begin)
    {
        fprintf(stream, "%*s| ", (int)offset, "");
        for (const char *i = line_begin; i < end; i++)
            fputc(i < begin ? ' ' : i == begin ? '^'
                                               : '~',
                  stream);
        fputc('\n', stream);
    }
}

/*
    Token definition
*/

const char *token_kind_str(token_kind_t kind)
{
    switch (kind)
    {
    case TOKEN_KIND_EOF:
        return "EOF";
    case TOKEN_KIND_ID:
        return "identifier";
    case TOKEN_KIND_LITERAL:
        return "literal";
    default:
        return "unknown";
    }
}

int token_is_token_kind(const char *what)
{
    uintptr_t kind = (uintptr_t)what;
    return kind < MAX_TOKEN_KIND;
}

token_kind_t token_to_token_kind(const char *what)
{
    return (uintptr_t)what;
}

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

/*
    Token parsing
*/
token_kind_t parse_token(const token_t *token, const char **begin, const char **end)
{

    static const char *keywords[] = {
        "if",
        "else",
        "while",
        "break",
        "continue",
        "return", NULL};

    static const char one_chars[] = {'(', ')', '[', ']', '{', '}', ',', ':', ';', '+', '-', '*', '/', '=', '&'};
    static char two_chars[][3] = {"+=", "-=", "*=", "/=", "=="};

    const char *p = *token;

    while (*p && is_space(*p))
        p++;

    if (!*p)
    {
        *begin = p;
        *end = p;
        return TOKEN_KIND_EOF;
    }
    else if (is_digit(*p))
    {
        *begin = p;
        while (*p && is_digit(*p))
            p++;
        *end = p;
        return TOKEN_KIND_LITERAL;
    }
    else if (is_id_char(*p))
    {
        *begin = p;
        while (*p && is_id_char(*p))
            p++;
        *end = p;

        for (size_t i = 0; keywords[i]; i++)
            if ((size_t)(*begin - *end) == strlen(keywords[i]) && strncmp(*begin, keywords[i], *end - *begin) == 0)
                return 0;

        return TOKEN_KIND_ID;
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
            return TOKEN_KIND_LITERAL;
        }
        else
        {
            print_location(stderr, p);
            fprintf(stderr, "error: Unexpected EOF. Expected closing `\"`.\n");
            print_snippet(stderr, p - 1, p);
            exit(1);
        }
    }
    else
    {
        for (size_t i = 0; i < sizeof(two_chars) / 3; i++)
            if (p[0] == two_chars[i][0] && p[1] == two_chars[i][1])
            {
                *begin = p;
                *end = p + 2;
                return 0;
            }
        for (size_t i = 0; i < sizeof(one_chars); i++)
            if (*p == one_chars[i])
            {
                *begin = p;
                *end = p + 1;
                return 0;
            }
        print_location(stderr, p);
        fprintf(stderr, "error: Unexpected char `%c`.\n", *p);
        print_snippet(stderr, p, p + 1);
        exit(1);
    }
}

/*
    Token matching
*/
int token_can_match(const token_t *token, const char *what)
{
    token_t tmp = *token;
    return token_match(&tmp, what);
}

int token_match(token_t *token, const char *what)
{
    const char *begin;
    const char *end;
    token_kind_t kind = parse_token(token, &begin, &end);

    if ((token_is_token_kind(what) && kind == token_to_token_kind(what)) ||
        (!token_is_token_kind(what) && (size_t)(end - begin) == strlen(what) && strncmp(begin, what, end - begin) == 0))
    {
        *token = end;
        return 1;
    }
    return 0;
}

void token_expect(token_t *token, const char *what)
{
    if (!token_match(token, what))
    {
        token_unexpected(token, token_is_token_kind(what) ? token_kind_str(token_to_token_kind(what)) : what);
        exit(1);
    }
}

void token_unexpected(const token_t *token, const char *expected)
{
    const char *begin = NULL;
    const char *end = NULL;
    token_kind_t kind = parse_token(token, &begin, &end);

    print_location(stderr, begin);
    if (kind == TOKEN_KIND_EOF)
        fprintf(stderr, "error: Unexpected token `EOF`. Expected %s.\n", expected);
    else
        fprintf(stderr, "error: Unexpected token `%.*s`. Expected %s.\n", (int)(end - begin), begin, expected);
    print_snippet(stderr, begin, end);
}

/*
    Emit functions
*/
void emit(const char *str, int skip)
{
    if (!skip)
        fprintf(output_file, "%s", str);
}

void emit_indented(const char *str, int skip, size_t indent)
{
    if (!skip)
        fprintf(output_file, "%*s", (int)indent * 4, str);
}

void emit_token(token_t *token, int skip)
{
    const char *begin = NULL;
    const char *end = NULL;
    (void)parse_token(token, &begin, &end);
    if (!skip)
        fprintf(output_file, "%.*s", (int)(end - begin), begin);
    *token = end;
}

void parse_type(token_t *token, int skip)
{

    if (token_can_match(token, TOKEN_ID))
    {
        emit_token(token, skip);
    }
    else if (token_match(token, "&"))
    {
        parse_type(token, skip);
        emit("*", skip);
    }
    else
    {
        token_unexpected(token, "type identifier or reference");
        exit(1);
    }
}

void parse_var_decl(token_t *token, int skip)
{
    token_t name = *token;
    token_expect(token, TOKEN_ID);
    token_expect(token, ":");
    parse_type(token, skip);
    emit(" ", skip);
    emit_token(&name, skip);
}

void parse_stmt(token_t *token, int skip, size_t indent)
{
    if (token_match(token, "if"))
    {
        emit_indented("if(", skip, indent);
        parse_expr(token, skip);
        emit(")\n", skip);
        parse_stmt_block(token, skip, indent);
        if (token_match(token, "else"))
        {
            emit_indented("else\n", skip, indent);
            parse_stmt_block(token, skip, indent);
        }
    }
    else if (token_can_match(token, "{"))
    {
        parse_stmt_block(token, skip, indent);
    }
    else
    {
        emit_indented("", skip, indent);
        token_t tmp = *token;
        if (token_match(&tmp, TOKEN_ID) && token_match(&tmp, ":"))
        {
            parse_var_decl(token, skip);
            token_expect(token, "=");
            if (!token_match(token, "undefined"))
            {
                emit(" = ", skip);
                parse_expr(token, skip);
            }
        }
        else
        {
            parse_expr(token, skip);
        }
        token_expect(token, ";");
        emit(";\n", skip);
    }
}

void parse_stmt_block(token_t *token, int skip, size_t indent)
{
    token_expect(token, "{");
    emit_indented("{\n", skip, indent);

    while (!token_match(token, "}"))
    {
        parse_stmt(token, skip, indent + 1);
    }

    emit_indented("}\n", skip, indent);
}

void parse_expr(token_t *token, int skip)
{
    token_t tmp = *token;
    token_expect(&tmp, TOKEN_LITERAL);
    emit_token(token, skip);
}
