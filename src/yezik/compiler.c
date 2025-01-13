#include "compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "generator.h"

#define TRY_EX(expr, msg) \
    if (expr)             \
    {                     \
        msg;              \
        goto error;       \
    }

#define TRY_ERRNO(expr, msg) \
    TRY_EX(expr, perror(msg))

#define TRY(expr, ...) \
    TRY_EX(expr, fprintf(stderr, __VA_ARGS__))

static char *read_file_(const char *path, size_t *size)
{
    size_t tmp = 0;
    if (!size)
        size = &tmp;
    int err = 0;
    char *buff = NULL;
    FILE *file = NULL;

    TRY_ERRNO((file = fopen(path, "rb")), path);
    TRY_ERRNO(fseek(file, 0, SEEK_END), "fseek");
    TRY_ERRNO((*size = (size_t)ftell(file)) == (size_t)-1, "ftell");
    TRY_ERRNO(fseek(file, 0, SEEK_SET), "fseek");
    TRY((buff = malloc(*size + 1)), "Out of memory\n")
    TRY_ERRNO(fread(buff, 1, *size, file) < *size, "fread");
    buff[*size] = '\0';

cleanup:

    if (file)
        fclose(file);
    if (err)
    {
        buff = NULL;
        free(buff);
    }

    return buff;
error:
    err = 1;
    goto cleanup;
}

static int gen_asm_(const char *out_file, AstGlobal *global)
{
    int err = 0;
    EmitCtx emitter = {0};
    FILE *out = NULL;

    TRY((out = fopen(out_file, "wb")), "");

    emit_ctx_init(&emitter, out);

    TRY(gen_ast_global(&emitter, global), "");

cleanup:
    emit_ctx_deinit(&emitter);
    if (out)
        fclose(out);

    return err;
error:
    err = 1;
    goto cleanup;
}

int compile(size_t src_files_count, const char **src_files, const char *out_file)
{
    int err = 0;
    AstGlobal global = {0};
    char **sources = NULL;
    TRY((sources = calloc(src_files_count, sizeof(*sources))), "Out of memory.\n");
    for (size_t i = 0; i < src_files_count; i++)
    {
        TRY((sources[i] = read_file_(src_files[i], NULL)), "");
    }

    ast_global_init(&global);

    for (size_t i = 0; i < src_files_count; i++)
    {
        LexerCtx lexer;
        lexer_ctx_init(&lexer, src_files[i], sources[i]);
        TRY(parse_ast_global(&lexer, &global), "");
    }

    TRY(gen_asm_(out_file, &global), "");

cleanup:

    ast_global_deinit(&global);

    if (sources)
    {
        for (size_t i = 0; i < src_files_count; i++)
        {
            free(sources[i]);
        }
        free(sources);
    }

    return err;
error:
    err = 1;
    goto cleanup;
}
