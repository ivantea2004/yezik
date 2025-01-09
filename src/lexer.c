#include "lexer.h"
#include "error.h"
#include <inttypes.h>
#include <ctype.h>
#include <string.h>

#define LEXER_ERROR(...)                                                                                             \
    {                                                                                                                \
        fprintf(stderr, "%s:%" PRIuPTR ": ", (ctx)->file, 1 + error_find_line((ctx)->text, (ctx)->pos, NULL, NULL)); \
        fprintf(stderr, __VA_ARGS__);                                                                                \
        error_snippet(stderr, (ctx)->text, (ctx)->pos);                                                              \
    }

void lexer_ctx_init(LexerCtx *ctx, const char *file, const char *text)
{
    ctx->file = file;
    ctx->text = text;
    ctx->pos = 0;
    ctx->prev_pos = 0;
}

void lexer_ctx_deinit(LexerCtx *ctx)
{
    (void)ctx;
}

static int lexer_peek_impl_(LexerCtx *ctx, Token *token);

int lexer_peek(LexerCtx *ctx, Token *token)
{
    ctx->prev_pos = ctx->pos;
    return lexer_peek_impl_(ctx, token);
}

int lexer_get(LexerCtx *ctx, Token *token)
{
    int res = lexer_peek(ctx, token);
    if (res <= 0)
    {
        return res;
    }
    else
    {
        LEXER_ERROR("Unexpected EOF.\n");
        return -1;
    }
}

void lexer_unget(LexerCtx *ctx)
{
    YEZIK_ASSERT(ctx->prev_pos < ctx->pos, "There is no token to unget.\n");
    ctx->prev_pos = ctx->pos;
}

// Skips whitespace and comments
static void lexer_skip_whitespace_(LexerCtx *ctx)
{
    while (ctx->text[ctx->pos] != '\0')
    {
        if (isspace(ctx->text[ctx->pos]))
        {
            ctx->pos++;
        }
        else if (strncmp(ctx->text + ctx->pos, "//", 2) == 0)
        {
            while (ctx->text[ctx->pos] != '\0' && ctx->text[ctx->pos] != '\n')
            {
                ctx->pos++;
            }
        }
        else if (strncmp(ctx->text + ctx->pos, "/*", 2) == 0)
        {
            ctx->pos += 2;
            while (ctx->text[ctx->pos] != '\0')
            {
                if (strncmp(ctx->text + ctx->pos, "*/", 2) == 0)
                {
                    ctx->pos += 2;
                    break;
                }
                ctx->pos++;
            }
        }
        else
        {
            return;
        }
    }
}

static int lexer_match_sign_(LexerCtx *ctx, Token *token);
static int lexer_match_number_(LexerCtx *ctx, Token *token);
static int lexer_match_keywork_or_id_(LexerCtx *ctx, Token *token);
static int lexer_match_string_(LexerCtx *ctx, Token *token);
static int lexer_match_char_(LexerCtx *ctx, Token *token);

/*
    Signs can be not separated by whitespace
    After literals and keywords whitespace or signs are expected
*/
int lexer_peek_impl_(LexerCtx *ctx, Token *token)
{

    lexer_skip_whitespace_(ctx);
    token->begin = ctx->pos;
    if (ctx->text[ctx->pos] == '\0')
    {
        if (ctx->pos == 0)
        {
            LEXER_ERROR("File is empty.\n");
            return -1;
        }
        else if (ctx->text[ctx->pos - 1] != '\n')
        {
            LEXER_ERROR("No newline at end of file.\n");
            return -1;
        }
        return 1;
    }
    else if (isdigit(ctx->text[ctx->pos]))
    {
        if (lexer_match_number_(ctx, token))
            return -1;
    }
    else if (isalpha(ctx->text[ctx->pos]))
    {
        if (lexer_match_keywork_or_id_(ctx, token))
            return -1;
    }
    else if (ctx->text[ctx->pos] == '"')
    {
        if (lexer_match_string_(ctx, token))
            return -1;
    }
    else if (ctx->text[ctx->pos] == '\'')
    {
        if (lexer_match_char_(ctx, token))
            return -1;
    }
    else
    {
        if (lexer_match_sign_(ctx, token))
            return -1;
    }
    token->end = ctx->pos;
    token->file = ctx->file;
    token->text = ctx->text;

    return 0;
}

int lexer_match_sign_(LexerCtx *ctx, Token *token)
{

#define M(what, e)                                                    \
    if (strncmp(ctx->text + ctx->pos, (what), sizeof(what) - 1) == 0) \
    {                                                                 \
        ctx->pos += sizeof(what) - 1;                                 \
        token->type = (e);                                            \
        return 0;                                                     \
    }
    M("(", TOKEN_PAR_OPEN);
    M(")", TOKEN_PAR_CLOSED);
    M("(", TOKEN_PAR_OPEN);
    M(")", TOKEN_PAR_CLOSED);
    M("[", TOKEN_BR_OPEN);
    M("]", TOKEN_BR_CLOSED);
    M("{", TOKEN_CUR_OPEN);
    M("}", TOKEN_CUR_CLOSED);

    M("+", TOKEN_PLUS);
    M("-", TOKEN_MINUS);
    M("*", TOKEN_MULT);
    M("/", TOKEN_DIV);
    M("%", TOKEN_REM);

#undef M
    LEXER_ERROR("Unexpected char.\n");

    return -1;
}

int lexer_match_number_(LexerCtx *ctx, Token *token)
{
    while (ctx->text[ctx->pos] != '\0' && isdigit(ctx->text[ctx->pos]))
    {
        ctx->pos++;
    }
    if (isalpha(ctx->text[ctx->pos]))
    {
        LEXER_ERROR("Numbers can not contain letters.\n");
        return -1;
    }
    token->type = TOKEN_INT;
    return 0;
}

static int lexer_match_id_(LexerCtx *ctx, Token *token)
{
    while (ctx->text[ctx->pos] != '\0' && isalnum(ctx->text[ctx->pos]))
    {
        ctx->pos++;
    }
    token->type = TOKEN_ID;
    return 0;
}

int lexer_match_keywork_or_id_(LexerCtx *ctx, Token *token)
{
    size_t begin = ctx->pos;
    if (lexer_match_id_(ctx, token))
    {
        return -1;
    }
    size_t end = ctx->pos;

#define KW(keyword, e)                                                       \
    if (end - begin == sizeof(keyword) - 1)                                  \
    {                                                                        \
        if (strncmp(ctx->text + begin, (keyword), sizeof(keyword) - 1) == 0) \
        {                                                                    \
            token->type = (e);                                               \
            return 0;                                                        \
        }                                                                    \
    }

    KW("let", TOKEN_LET);
    KW("fn", TOKEN_FN);

#undef KW
    return 0;
}

static int lexer_match_string_(LexerCtx *ctx, Token *token)
{
    (void)ctx;
    (void)token;
    return -1;
}

static int lexer_match_char_(LexerCtx *ctx, Token *token)
{
    (void)ctx;
    (void)token;
    return -1;
}
