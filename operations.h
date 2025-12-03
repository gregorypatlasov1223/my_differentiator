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

void free_subtree(node_t* node);
size_t count_tree_nodes(node_t* node);
tree_error_type evaluate_tree(tree_t* tree, variable_table* var_table, double* result);
tree_error_type differentiate_tree(tree_t* tree, const char* variable_name, tree_t* result_tree);
node_t* create_node(node_type type, value_of_tree_element data, node_t* left, node_t* right);
node_t* create_node_from_token(const char* token, node_t* parent);
tree_error_type optimize_tree_with_dump(tree_t* tree, FILE* tex_file, variable_table* var_table);


#endif // OPERATIONS_H_
