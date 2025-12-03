#ifndef PROCESSING_DIFF_H
#define PROCESSING_DIFF_H

#include <stdio.h>

#include "tree_base.h"
#include "variable_parse.h"
#include "tree_error_types.h"

struct differentiator_struct
{
    tree_t tree;
    variable_table var_table;
    char* expression;
    FILE* tex_file;
    double result;
};

differentiator_struct* create_differentiator_struct();
void destroy_differentiator_struct(differentiator_struct* diff_struct);

tree_error_type initialize_expression         (differentiator_struct* diff_struct, int argc, const char** argv);
tree_error_type parse_expression_tree          (differentiator_struct* diff_struct);
tree_error_type initialize_latex_output        (differentiator_struct* diff_struct);
tree_error_type request_variable_values        (differentiator_struct* diff_struct);
tree_error_type evaluate_original_function     (differentiator_struct* diff_struct);
tree_error_type optimize_expression_tree       (differentiator_struct* diff_struct);
tree_error_type plot_function_graph            (differentiator_struct* diff_struct);
tree_error_type perform_differentiation_process(differentiator_struct* diff_struct);
tree_error_type finalize_latex_output          (differentiator_struct* diff_struct);
void print_error_and_cleanup(differentiator_struct* diff_struct, tree_error_type error);


#endif // PROCESSING_DIFF_H
