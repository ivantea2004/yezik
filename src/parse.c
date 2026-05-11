#include "parse.h"
#include "helpers.h"
#include "import.h"
#include "error.h"

void parse_type(token_t *token, out_t out);
void parse_var_decl(token_t *token, out_t out);

void parse_function(token_t *token, out_t out, int def);
void parse_var(token_t *token, out_t out, int def);
void parse_const(token_t *token, out_t out, int def);

void parse_stmt(token_t *token, out_t out, int locals_pass, size_t indent);

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
    expect(token, TOKEN_FUNCTION);
    token_t name = *token;
    token_t sig_end;
    for (int return_type = 1; return_type >= 0; return_type--, sig_end = *token, *token = name)
    {
        output_token(token, return_type ? NULL : out);
        output("(", return_type ? NULL : out);
        expect(token, TOKEN_ID);
        expect(token, TOKEN_O_PAR);
        if (match(token, TOKEN_C_PAR))
        {
            output("void)", return_type ? NULL : out);
        }
        else
        {
            while (1)
            {
                parse_var_decl(token, return_type ? NULL : out);
                if (match(token, TOKEN_C_PAR))
                    break;
                expect(token, TOKEN_COMMA);
                output(", ", return_type ? NULL : out);
            }
            output(")", return_type ? NULL : out);
        }
        if (can_match(token, TOKEN_SEMI) || can_match(token, TOKEN_O_CUR))
            output("void", return_type ? out : NULL);
        else
            parse_type(token, return_type ? out : NULL);
        output(" ", return_type ? out : NULL);
    }
    *token = sig_end;

    if (match(token, TOKEN_SEMI))
    {
        output(";\n", out);
        return;
    }

    if (!def)
    {
        output(";\n", out);
        out = NULL;
    }

    output("\n{\n", out);

    token_t body_begin = *token;
    token_t body_end;
    for (int locals_pass = 1; locals_pass >= 0; locals_pass--, body_end = *token, *token = body_begin)
    {
        expect(token, TOKEN_O_CUR);
        while (!match(token, TOKEN_C_CUR))
            parse_stmt(token, out, locals_pass, 1);
    }
    *token = body_end;
    output("}\n", out);
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

void parse_stmt(token_t *token, out_t out, int locals_pass, size_t indent)
{
    if (match(token, TOKEN_O_CUR))
    {
        output_indented("{\n", locals_pass ? NULL : out, locals_pass ? 1 : indent);
        while (!match(token, TOKEN_C_CUR))
            parse_stmt(token, out, locals_pass, indent + 1);
        output_indented("}\n", locals_pass ? NULL : out, locals_pass ? 1 : indent);
    }
    else
    {

        token_t name = *token;
        if (match(&name, TOKEN_ID) && match(&name, TOKEN_COLON))
        {
            name = *token;
            output_indented("", locals_pass ? out : NULL, 1);
            parse_var_decl(token, locals_pass ? out : NULL);
            output(";\n", locals_pass ? out : NULL);
            expect(token, TOKEN_ASSIGN);
            if (!match(token, TOKEN_UNDEFINED))
            {
                output_indented("", locals_pass ? NULL : out, indent);
                output_token(&name, locals_pass ? NULL : out);
                output(" = ", locals_pass ? NULL : out);
                parse_expr(token, locals_pass ? NULL : out);
                output(";\n", locals_pass ? NULL : out);
            }
            expect(token, TOKEN_SEMI);
        }
        else
        {
            output_indented("", locals_pass ? NULL : out, indent);
            parse_expr(token, locals_pass ? NULL : out);
            expect(token, TOKEN_SEMI);
            output(";\n", locals_pass ? NULL : out);
        }
    }
}

void parse_expr(token_t *token, out_t out)
{
    token_t value = *token;
    expect(token, TOKEN_INT);
    output_token(&value, out);
}
