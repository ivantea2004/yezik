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
/*                                 Path utils                                 */
/* -------------------------------------------------------------------------- */

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

/* -------------------------------------------------------------------------- */
/*                                 str_list_t                                 */
/* -------------------------------------------------------------------------- */

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
    list->strs[list->size] = strdup(str);
    list->size++;
}

/* -------------------------------------------------------------------------- */
/*                                   Globals                                  */
/* -------------------------------------------------------------------------- */

char *current_path;
char *current_text;
FILE *output_file;
str_list_t imported_paths;

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
    X(TOKEN_FALSE, "false")

#define TOKEN_KEYWORDS_X(X)       \
    X(TOKEN_IF, "if")             \
    X(TOKEN_ELSE, "else")         \
    X(TOKEN_WHILE, "while")       \
    X(TOKEN_BREAK, "break")       \
    X(TOKEN_CONTINUE, "continue") \
                                  \
    X(TOKEN_IMPORT, "import")     \
    X(TOKEN_CONST, "const")       \
    X(TOKEN_LET, "let")           \
    X(TOKEN_TYPE, "type")         \
    X(TOKEN_RECORD, "record")     \
    X(TOKEN_ENUM, "enum")

#define TOKEN_SYMBOLS_X(X) \
    X(TOKEN_ASSIGN, "=")   \
                           \
    X(TOKEN_COMMA, ",")    \
    X(TOKEN_COLON, ":")    \
    X(TOKEN_SEMI, ";")     \
                           \
    X(TOKEN_LPAR, "(")     \
    X(TOKEN_RPAR, ")")     \
    X(TOKEN_LCUR, "{")     \
    X(TOKEN_RCUR, "}")     \
    X(TOKEN_LBR, "[")      \
    X(TOKEN_RBR, "}")      \
                           \
    X(TOKEN_PLUS, "+")

#define TOKEN(x, ...) x,

typedef enum
{
    TOKEN_EOF,
    TOKEN_ID,
    TOKEN_INT,
    TOKEN_STR,
    TOKEN_BUILTINS_X(TOKEN)
    TOKEN_KEYWORDS_X(TOKEN) TOKEN_SYMBOLS_X(TOKEN)
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
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
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

#undef HARDCODED
#undef KEYWORD
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

// static void output_token_char(const char *pos, int skip)
// {
//     const char *begin;
//     const char *end;
//     token_kind_t kind;
//     (void)lexer_get_token(pos, &begin, &end, &kind);
//     if (kind != TOKEN_STR)
//         panic();
//     if (!skip)
//         fprintf(output_file, "'%.*s'", (int)(end - begin - 2), begin + 1);
// }

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

static const char *parse_type(const char *pos, int skip)
{
    token_expect(pos, TOKEN_ID);
    output_token_id(pos, skip);
    return token_step(pos);
}

static const char *parse_expr(const char *pos, int skip)
{
    token_kind_t k = token_peek(pos);
    if (k == TOKEN_INT)
    {
        output_token_int(pos, skip);
        return token_step(pos);
    }
    else if (k == TOKEN_STR)
    {
        output_token_str(pos, skip);
        return token_step(pos);
    }
    token_unexpected(pos, "expression.\n");
    panic();
}

static const char *parse_stmt(const char *pos, int skip, size_t indent)
{
    const token_kind_t k = token_peek(pos);
    if (k == TOKEN_LCUR)
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
    else if (k == TOKEN_LET)
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
        output(";\n", skip);
        return token_expect(pos, TOKEN_SEMI);
    }
    else
    {
        token_unexpected(pos, "statement");
        panic();
    }
}

void parse_file(void)
{
    const char *pos = current_text;
    parse_stmt(pos, 0, 0);
    // // pos = lexer_skip_space(pos);
    // // parse_stmt(pos, 0, 0);
    // while (1)
    // {
    //     const char *begin;
    //     const char *end;
    //     token_kind_t kind;
    //     pos = lexer_get_token(pos, &begin, &end, &kind);
    //     if (kind == TOKEN_EOF)
    //         break;
    //     printf("%s\n", token_kind_str(kind));
    // }
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
        path = strdup(path);
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
