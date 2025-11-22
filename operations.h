#ifndef OPERATIONS_H_
#define OPERATIONS_H_

#include "tree_common.h"
#include "variable_parse.h"
#include "tree_error_types.h"

tree_error_type evaluate_tree_recursive(node_t* node, variable_table* var_table, double* result);
tree_error_type evaluate_tree(tree_t* tree, variable_table* var_table, double* result);

#endif // OPERATIONS_H_
