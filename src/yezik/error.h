/*

    Error handling and reporting module of compiler

 */
#pragma once
#include <yezik/yezik.h>>

/*
    Prints a line containing <place> and hightlights <place>
*/
void error_snippet(FILE *out, const char *text, size_t place);

/*
    Finds line which contains <place>
    In <begin> and <end> line borders are written
    Returns line's number starting from 0
*/
size_t error_find_line(const char *text, size_t place, size_t *begin, size_t *end);
