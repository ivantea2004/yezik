#include "compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "generator.h"

static char *read_file_(FILE *file, size_t *size)
{
    fseek(file, 0, SEEK_END);
    *size = (size_t)ftell(file);
    fseek(file, 0, SEEK_SET);
    char *buff = malloc(*size + 1);
    if (!buff)
    {
        return NULL;
    }
    fread(buff, 1, *size, file);
    buff[*size] = '\0';
    return buff;
}

static int parse_file_(const char *src_path, AstGlobal *global)
{
    FILE *src_file = fopen(src_path, "r");
    if (!src_file)
    {
        perror(src_path);
        return 1;
    }
    size_t src_size = 0;
    char *src = read_file_(src_file, &src_size);
    fclose(src_file);
    if (!src)
    {
        return 1;
    }

    LexerCtx lexer;
    lexer_ctx_init(&lexer, src_path, src);

    int err = parse_ast_global(&lexer, global);

    lexer_ctx_deinit(&lexer);
    free(src);
    return err;
}

int compile_file(const char *src_path, const char *out_path)
{

    AstGlobal global;
    ast_global_init(&global);

    int err = parse_file_(src_path, &global);

    if (!err)
    {
        ast_global_dump(stdout, &global);
        EmitCtx emitter;

        FILE *out = fopen(out_path, "w");
        if (!out)
        {
            perror(out_path);
            err = 1;
        }
        else
        {

            emit_ctx_init(&emitter, out);

            err = gen_ast_global(&emitter, &global);
            fclose(out);
            emit_ctx_deinit(&emitter);
        }
    }

    ast_global_deinit(&global);

    return err;
}
