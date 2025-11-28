#ifndef TREE_USER_INTERFACE_H_
#define TREE_USER_INTERFACE_H_

#include "tree_common.h"
#include "variable_parse.h"
#include "tree_error_types.h"

const char* get_data_base_filename(int argc, const char** argv);
const char* tree_error_translator(tree_error_type error);
char* select_differentiation_variable(variable_table* var_table);
void print_tree_error(tree_error_type error);

#endif // TREE_USER_INTERFACE_H_
