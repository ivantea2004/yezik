#include "parse.h"
#include "helpers.h"
#include "import.h"
#include "error.h"

void parse_type(token_t *token, out_t out);
void parse_var_decl(token_t *token, out_t out);

void parse_function(token_t *token, out_t out, int def);
void parse_var(token_t *token, out_t out, int def);
void parse_const(token_t *token, out_t out, int def);

void parse_expr(token_t *token, out_t out);

void parse_file(FILE *out)
{

    for (int def = 0; def < 2; def++)
    {
        const char *token = current_source_file_text();

        while (1)
        {
            token_kind_t k = peek(&token);
            if (k == TOKEN_EOF)
                break;
            else if (k == TOKEN_FUNCTION)
                parse_function(&token, out, def);
            else if (k == TOKEN_CONST)
                parse_const(&token, out, def);
            else
                parse_var(&token, out, def);
        }
    }
}

void parse_type(token_t *token, out_t out)
{
    if (can_match(token, TOKEN_ID))
    {
        output_token(token, out);
        match(token, TOKEN_ID);
    }
    else
    {
        unexpected_token(token, "type");
        terminate();
    }
}

void parse_function(token_t *token, out_t out, int def)
{
    (void)token;
    (void)out;
    (void)def;
}

void parse_var_decl(token_t *token, out_t out)
{
    token_t name = *token;
    expect(token, TOKEN_ID);
    expect(token, TOKEN_COLON);
    parse_type(token, out);
    output(" ", out);
    output_token(&name, out);
}

void parse_var(token_t *token, out_t out, int def)
{

    output("extern ", def ? NULL : out);
    parse_var_decl(token, out);
    expect(token, TOKEN_ASSIGN);
    if (!match(token, TOKEN_UNDEFINED))
    {
        output(" = ", def ? out : NULL);
        parse_expr(token, def ? out : NULL);
    }
    expect(token, TOKEN_SEMI);
    output(";\n", out);
}

void parse_const(token_t *token, out_t out, int def)
{
    if (def)
        out = NULL;
    expect(token, TOKEN_CONST);
    token_t name = *token;
    expect(token, TOKEN_ID);
    output("#define ", out);
    output_token(&name, out);
    output(" ", out);
    expect(token, TOKEN_ASSIGN);
    parse_expr(token, out);
    expect(token, TOKEN_SEMI);
    output("\n", out);
}

void parse_expr(token_t *token, out_t out)
{
    token_t value = *token;
    expect(token, TOKEN_INT);
    output_token(&value, out);
}
