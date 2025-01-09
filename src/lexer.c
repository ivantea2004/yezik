#include "lexer.h"
#include "error.h"
#include <inttypes.h>

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

int lexer_peek_impl_(LexerCtx *ctx, Token *token)
{
    (void)ctx;
    (void)token;
    return -1;
}
