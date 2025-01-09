#include "compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include <unistd.h>
static char* read_file_(FILE*file, size_t*size)
{
    fseek(file, 0, SEEK_END);
    *size = (size_t)ftell(file);
    fseek(file, 0, SEEK_SET);
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
    char buff[100];
    getcwd(buff, 100);
    printf("%s\n", buff);
    FILE*src_file = fopen(src_path, "rb");
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
        if (res < 0 ) {
            printf("Error happend!\n");
            break;
        } else if(res > 0) {
            printf("EOF\n");
            break;
        }

        {
            char c = token.text[token.end];
            src[token.end] = '\0';
            printf("%s\n", token.text + token.begin);
            src[token.end] = c;
        }

    }

    lexer_ctx_deinit(&lexer);

    free(src);

    (void)out_path;

    return 0;
}
