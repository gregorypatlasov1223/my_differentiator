#ifndef LATEX_DUMP_H
#define LATEX_DUMP_H

#include "tree_base.h"
#include "variable_parse.h"


// const int MAX_PARTS = 50;


// int is_binary_operator(const char* expression, int i, int length);
// void add_part(char** parts, int* part_count, const char* expression, int start, int end);
// int split_by_operators(const char* expression, char** parts, int* part_count,
//                       const char* operators);
// char** split_expression(const char* expression, int* part_count);
// void free_parts(char** parts, int part_count);
void tree_to_string_simple(node_t* node, char* buffer, size_t* position, int buffer_size);
tree_error_type dump_original_function(FILE* file, tree_t* tree, double result_value);
tree_error_type dump_derivative(FILE* file, tree_t* derivative_tree, double derivative_result, int derivative_order);
tree_error_type dump_variable_table(FILE* file, variable_table* var_table);
tree_error_type generate_latex_dump(tree_t* tree, variable_table* var_table, const char* filename, double result_value);
tree_error_type generate_latex_dump_with_derivatives(tree_t* tree, tree_t** derivative_trees, double* derivative_results,
                                              int derivative_count, variable_table* var_table,
                                              const char* filename, double result_value);

#endif // LATEX_DUMP_H
