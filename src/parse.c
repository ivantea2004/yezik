#include "parse.h"
#include "helpers.h"
#include "import.h"
#include "error.h"

void parse_type(token_t *token, out_t out)
{
    if (match(token, TOKEN_ID))
    {
        output_token(token, out);
        match(token, ANY_TOKEN);
    }
    else
    {
        unexpected_token(token, "type");
        terminate();
    }
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

void parse_file(FILE *out)
{

    const char *i = current_source_file_text();

    while (*i)
    {
        parse_var_decl(&i, out);
    }
    printf("EOF\n");

    (void)out;
}
