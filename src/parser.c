#include "parser.h"
#include "error.h"
#include <string.h>
#include <stdlib.h>

#define TOKEN_ERROR(token, ...)                                                                                           \
    {                                                                                                                     \
        fprintf(stderr, "%s:%" PRIuPTR ": ", (token).file, 1 + error_find_line((token).text, (token).begin, NULL, NULL)); \
        fprintf(stderr, __VA_ARGS__);                                                                                     \
        error_snippet(stderr, (token).text, (token).begin);                                                               \
    }

static char *token_id_name_(Token *token)
{
    YEZIK_ASSERT(token->type == TOKEN_ID, "Only identifiers have names.\n");
    size_t len = token->end - token->begin;
    char *buff = malloc(len + 1);
    memcpy(buff, token->text + token->begin, len);
    buff[len] = '\0';
    return buff;
}

// static Token expect_token_(LexerCtx*lexer, TokenType type);

#define EXPECT_TOKEN_AND_DECL(name, t)                \
    Token name;                                       \
    {                                                 \
        lexer_get((lexer), &name);                    \
        if (name.type != t)                           \
        {                                             \
            TOKEN_ERROR(name, "Unexpected token.\n"); \
            return 1;                                 \
        }                                             \
    }

#define EXPECT_TOKEN(type)                \
    {                                     \
        EXPECT_TOKEN_AND_DECL(tmp, type); \
    }

static int parse_ast_fn_def_(LexerCtx *lexer, AstFnDef *fn)
{
    EXPECT_TOKEN(TOKEN_FN);
    EXPECT_TOKEN_AND_DECL(name, TOKEN_ID);
    EXPECT_TOKEN(TOKEN_PAR_OPEN);
    EXPECT_TOKEN(TOKEN_PAR_CLOSED);

    EXPECT_TOKEN(TOKEN_CUR_OPEN);
    EXPECT_TOKEN(TOKEN_CUR_CLOSED);

    fn->name = token_id_name_(&name);
    return 0;
}

int parse_ast_global(LexerCtx *lexer, AstGlobal *global)
{
    while (1)
    {
        Token token;
        int res = lexer_peek(lexer, &token);
        if (res > 0)
        {
            break;
        }
        else if (res < 0)
        {
            return -1;
        }
        else
        {
            lexer_unget(lexer);
            if (token.type == TOKEN_FN)
            {
                AstFnDef *fn = ast_global_add_fn(global);
                if (parse_ast_fn_def_(lexer, fn))
                    return 1;
            }
            else
            {
                TOKEN_ERROR(token, "Unexpected token.\n");
                return 1;
            }
        }
    }
    return 0;
}
