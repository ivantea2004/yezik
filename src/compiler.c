#include "compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"

static char* read_file_(FILE*file, size_t*size)
{
    fseek(file, SEEK_END, 0);
    *size = (size_t)ftell(file);
    fseek(file, SEEK_SET, 0);
    char *buff = malloc(*size + 1);
    if (!buff) {
        return NULL;
    }
    fread(buff, 1, *size, file);
    buff[*size] = '\0';
    return buff;
}

int compile_file(const char *src_path, const char *out_path)
{
    
    FILE*src_file = fopen(src_path, "r");
    if (!src_file) {
        perror(src_path);
        return 1;
    }
    size_t src_size = 0;
    char *src = read_file_(src_file, &src_size);
    fclose(src_file);
    if (!src) {
        return 1;
    }

    LexerCtx lexer;
    lexer_ctx_init(&lexer, src_path, src);

    while (1) {
        Token token;
        int res = lexer_peek(&lexer, &token);
        if (res != 0 ) {
            break;
        }
        
    }

    lexer_ctx_deinit(&lexer);

    free(src);

    (void)out_path;

    return 0;
}
