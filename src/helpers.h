#pragma once
#include <stdio.h>
#include "token.h"

#define ANY_TOKEN ((token_kind_t)(-1))

typedef const char *token_t;

void unexpected_token(const token_t *token, const char *expected);
token_kind_t peek(const token_t *token);
int match(token_t *token, token_kind_t expected_kind);
int can_match(const token_t *token, token_kind_t expected_kind);
void expect(token_t *token, token_kind_t expected_kind);

typedef FILE *out_t;

void output(const char *s, out_t out);
void output_indented(const char *s, size_t indent, out_t out);
void output_token(const token_t *token, out_t out);
