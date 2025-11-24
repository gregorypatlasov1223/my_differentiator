#include <math.h>
#include <stdio.h>

#include "operations.h"
#include "tree_common.h"
#include "variable_parse.h"
#include "tree_error_types.h"

tree_error_type evaluate_tree_recursive(node_t* node, variable_table* var_table, double* result)
{
    if (node == NULL || result == NULL || var_table == NULL)
        return TREE_ERROR_NULL_PTR;

    switch(node -> type)
    {
        case NODE_NUM:
            *result = node -> data.num_value;
            return TREE_ERROR_NO;

        case NODE_OP:
            {
                double left_result = 0, right_result = 0;
                tree_error_type error = TREE_ERROR_NO;

                if (node -> right == NULL)
                    return TREE_ERROR_NULL_PTR;

                // для бинарных операций вычисляем оба аргумента
                if (node -> data.op_value != OP_SIN && node -> data.op_value != OP_COS)
                {
                    if (node -> left == NULL)
                        return TREE_ERROR_NULL_PTR;

                    error = evaluate_tree_recursive(node -> left, var_table, &left_result);
                    if (error != TREE_ERROR_NO) return error;
                }

                error = evaluate_tree_recursive(node -> right, var_table, &right_result);
                if (error != TREE_ERROR_NO) return error;

                switch(node -> data.op_value)
                {
                    case OP_ADD:
                        *result = left_result + right_result;
                        break;
                    case OP_SUB:
                        *result = left_result - right_result;
                        break;
                    case OP_MUL:
                        *result = left_result * right_result;
                        break;
                    case OP_DIV:
                        if (fabs(right_result) < 1e-10)
                            return TREE_ERROR_DIVISION_BY_ZERO;
                        *result = left_result / right_result;
                        break;
                    case OP_SIN:
                        *result = sin(right_result);
                        break;
                    case OP_COS:
                        *result = cos(right_result);
                        break;
                    default:
                        return TREE_ERROR_UNKNOWN_OPERATION;
                }
                return TREE_ERROR_NO;
            }

        case NODE_VAR:
            {
                const char* var_name = node -> data.var_definition.name;

                double value = 0.0;
                tree_error_type error = get_variable_value(var_table, var_name, &value);

                if (error == TREE_ERROR_VARIABLE_NOT_FOUND)
                {
                    error = add_variable(var_table, var_name);
                    if (error != TREE_ERROR_NO) return error;

                    printf("Variable '%s' not defined.\n", var_name);
                    error = request_variable_value(var_table, var_name);
                    if (error != TREE_ERROR_NO) return error;

                    error = get_variable_value(var_table, var_name, &value);
                    if (error != TREE_ERROR_NO) return error;
                }
                else if (error == TREE_ERROR_VARIABLE_UNDEFINED)
                {
                    printf("Variable '%s' not defined.\n", var_name);
                    error = request_variable_value(var_table, var_name);
                    if (error != TREE_ERROR_NO) return error;

                    error = get_variable_value(var_table, var_name, &value);
                    if (error != TREE_ERROR_NO) return error;
                }
                else if (error != TREE_ERROR_NO)
                {
                    return error;
                }

                *result = value;
                return TREE_ERROR_NO;
            }
        default:
            return TREE_ERROR_UNKNOWN_OPERATION;
    }
}

tree_error_type evaluate_tree(tree_t* tree, variable_table* var_table, double* result)
{
    if (tree == NULL || result == NULL || var_table == NULL)
        return TREE_ERROR_NULL_PTR;

    if (tree -> root == NULL)
        return TREE_ERROR_NULL_PTR;

    return evaluate_tree_recursive(tree -> root, var_table, result);
}


node_t* create_node(node_type type, value_of_tree_element data, node_t* left, node_t* right)
{
    node_t* node = (node_t*)calloc(1, sizeof(node_t));
    if (node == NULL)
        return NULL;

    node -> type   = type;
    node -> left   = left;
    node -> right  = right;
    node -> parent = NULL;

    node -> data   = data;

    if (type == NODE_VAR && data.var_definition.name)
    {
        node -> data.var_definition.name = data.var_definition.name;
        node -> data.var_definition.hash = compute_hash(data.var_definition.name);
    }

    if (left != NULL)
        left -> parent = node;
    if (right != NULL)
        right -> parent = node;

    return node;
}


node_t* copy_node(node_t* original)
{
    assert(original != NULL);

    value_of_tree_element data = {};

    switch(original -> type)
    {
        case NODE_NUM:
            data.num_value = original -> data.num_value;
            return create_node(NODE_NUM, data, NULL, NULL);

        case NODE_VAR:
            data.var_definition.hash = original -> data.var_definition.hash;
            if (original -> data.var_definition.name != NULL)
            {
                data.var_definition.name = strdup(original -> data.var_definition.name);
            }
            else
            {
                data.var_definition.name = NULL;
            }            return create_node(NODE_VAR, data, NULL, NULL);

        case NODE_OP:
            data.op_value = original -> data.op_value;
            return create_node(NODE_OP, data, copy_node(original -> left), copy_node(original -> right));
        default:
            return NULL;

    }
}
