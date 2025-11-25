#ifndef OPERATIONS_H_
#define OPERATIONS_H_

#include "tree_common.h"
#include "variable_parse.h"
#include "tree_error_types.h"

struct operator_mapping
{
    const char* string;
    operation_type op_type;
};

const int OPERATORS_COUNT = 6;
extern const operator_mapping OPERATORS[];

tree_error_type evaluate_tree_recursive(node_t* node, variable_table* var_table, double* result);
tree_error_type evaluate_tree(tree_t* tree, variable_table* var_table, double* result);
node_t* create_node(node_type type, value_of_tree_element data, node_t* left, node_t* right);
node_t* copy_node(node_t* original);
node_t* differentiate_node(node_t* node, const char* variable_name);
size_t count_tree_nodes(node_t* node);
tree_error_type differentiate_tree(tree_t* tree, const char* variable_name, tree_t* result_tree);
node_t* create_node_from_token(const char* token, node_t* parent);


#endif // OPERATIONS_H_
