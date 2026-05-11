#pragma once
#include <stdio.h>

void import_file(const char *path, FILE *out);
const char *current_source_file_path(void);
const char *current_source_file_text(void);
