#ifndef LATEX_DUMP_H
#define LATEX_DUMP_H

#include <stdio.h>

#include "tree_base.h"
#include "variable_parse.h"


void          tree_to_string_simple(node_t* node, char* buffer, int* pos, int buffer_size);
char*         convert_latex_to_PGF_plot(const char* latex_expr);

tree_error_type start_latex_dump(FILE* file);
tree_error_type end_latex_dump(FILE* file);
tree_error_type add_latex_plot(FILE* file, const char* function_formula,
                          double x_min, double x_max, const char* title);
tree_error_type dump_original_function_to_file(FILE* file, tree_t* tree, double result_value);
tree_error_type dump_optimization_step_to_file(FILE* file, const char* description, tree_t* tree, double result_value);
tree_error_type dump_derivative_to_file(FILE* file, tree_t* derivative_tree, double derivative_result, int derivative_order);
tree_error_type dump_variable_table_to_file(FILE* file, variable_table* var_table);

#endif // LATEX_DUMP_H
