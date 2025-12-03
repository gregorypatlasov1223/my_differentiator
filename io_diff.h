#ifndef IO_DIFF_H_
#define IO_DIFF_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "tree_base.h"
#include "tree_error_types.h"

const int COEFFICIENT = 2;

char* read_expression_from_file(const char* filename);
size_t get_file_size(FILE* file);
void skip_spaces(const char* buffer, size_t* pos);

#endif //IO_DIFF_H_
