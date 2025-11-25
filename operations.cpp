#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "tree_base.h"
#include "operations.h"
#include "tree_common.h"
#include "variable_parse.h"
#include "tree_error_types.h"


const operator_mapping OPERATORS[] = {
    {"+", OP_ADD}, {"-", OP_SUB},   {"*", OP_MUL},
    {"/", OP_DIV}, {"sin", OP_SIN}, {"cos", OP_COS}
};


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

    if (left  != NULL)
        left -> parent = node;
    if (right != NULL)
        right -> parent = node;

    return node;
}


node_t* copy_node(node_t* original)
{
    if (original == NULL)
        return NULL;

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
            }
            return create_node(NODE_VAR, data, NULL, NULL);

        case NODE_OP:
            data.op_value = original -> data.op_value;
            return create_node(NODE_OP, data, copy_node(original -> left), copy_node(original -> right));
        default:
            return NULL;
    }
}


node_t* differentiate_node(node_t* node, const char* variable_name)
{
    if (node == NULL)
        return NULL;

    value_of_tree_element data = {};

    switch(node -> type)
    {
        case NODE_NUM:
            data.num_value = 0.0;
            return create_node(NODE_NUM, data, NULL, NULL);

        case NODE_VAR:
            if (node -> data.var_definition.name != NULL && strcmp(node -> data.var_definition.name, variable_name) == 0)
            {
                data.num_value = 1.0; // анализируем по какой переменной дифф
                return create_node(NODE_NUM, data, NULL, NULL);
            }
            else
            {
                data.num_value = 0.0;
                return create_node(NODE_NUM, data, NULL, NULL);
            }

        case NODE_OP:
            switch(node -> data.op_value)
            {
                case OP_ADD:
                case OP_SUB:
                {
                    // d(u +- v)/dx = du/dx +- dv/dx
                    node_t* left_deriv  = differentiate_node(node -> left, variable_name);
                    node_t* right_deriv = differentiate_node(node -> right, variable_name);
                    data.op_value = node -> data.op_value;

                    return create_node(NODE_OP, data, left_deriv, right_deriv);
                }

                case OP_MUL:
                {
                    // d(u*v)/dx = u*dv/dx + v*du/dx
                    node_t* u = copy_node(node -> left);
                    node_t* v = copy_node(node -> right);
                    node_t* du_dx = differentiate_node(node -> left, variable_name);
                    node_t* dv_dx = differentiate_node(node -> right, variable_name);

                    data.op_value = OP_MUL;

                    node_t* term1 = create_node(NODE_OP, data, u, dv_dx);
                    node_t* term2 = create_node(NODE_OP, data, v, du_dx);

                    data.op_value = OP_ADD;

                    return create_node(NODE_OP, data, term1, term2);
                }

                case OP_DIV:
                {
                    // d(u/v)/dx = (v*du/dx - u*dv/dx) / (v^2)
                    node_t* u = copy_node(node -> left);
                    node_t* v = copy_node(node -> right);
                    node_t* du_dx = differentiate_node(node -> left, variable_name);
                    node_t* dv_dx = differentiate_node(node -> right, variable_name);

                    data.op_value = OP_MUL;
                    node_t* numerator_term1 = create_node(NODE_OP, data, copy_node(v), du_dx);
                    node_t* numerator_term2 = create_node(NODE_OP, data, copy_node(u), dv_dx);

                    data.op_value = OP_SUB;
                    node_t* numerator  = create_node(NODE_OP, data, numerator_term1, numerator_term2);

                    data.op_value = OP_MUL;
                    node_t* v_squared = create_node(NODE_OP, data, copy_node(v), copy_node(v));

                    data.op_value = OP_DIV;
                    return create_node(NODE_OP, data, numerator, v_squared);
                }

                case OP_SIN:
                {
                    // d(sin(u))/dx = cos(u) * du/dx
                    node_t* u = copy_node(node -> right);
                    node_t* du_dx = differentiate_node(node -> right, variable_name);

                    data.op_value = OP_COS;
                    node_t* cos_u = create_node(NODE_OP, data, NULL, u);

                    data.op_value = OP_MUL;
                    return create_node(NODE_OP, data, cos_u, du_dx);
                }

                case OP_COS:
                {
                    // d(cos(u))/dx = -sin(u) * du/dx
                    node_t* u = copy_node(node -> right);
                    node_t* du_dx = differentiate_node(node -> right, variable_name);

                    data.op_value = OP_SIN;
                    node_t* sin_u = create_node(NODE_OP, data, NULL, u);

                    data.num_value = -1.0;
                    node_t* minus_one = create_node(NODE_NUM, data, NULL, NULL);

                    data.op_value = OP_MUL;
                    node_t* minus_sin_u = create_node(NODE_OP, data, minus_one, sin_u);

                    data.op_value = OP_MUL;
                    return create_node(NODE_OP, data, minus_sin_u, du_dx);
                }

                default:
                    data.num_value = 0.0;
                    return create_node(NODE_NUM, data, NULL, NULL);
            }

        default:
            data.num_value = 0.0;
            return create_node(NODE_NUM, data, NULL, NULL);
    }
}


size_t count_tree_nodes(node_t* node)
{
    if (node == NULL)
        return 0;

    return 1 + count_tree_nodes(node -> left) + count_tree_nodes(node -> right);
}


tree_error_type differentiate_tree(tree_t* tree, const char* variable_name, tree_t* result_tree)
{
    if (tree == NULL || variable_name == NULL || result_tree == NULL)
        return TREE_ERROR_NULL_PTR;

    if (tree -> root == NULL)
        return TREE_ERROR_NULL_PTR;

    node_t* derivative_root = differentiate_node(tree -> root, variable_name);
    if (derivative_root == NULL)
        return TREE_ERROR_ALLOCATION;

    result_tree -> root = derivative_root;
    result_tree -> size = count_tree_nodes(derivative_root);

    return TREE_ERROR_NO;
}


node_t* create_node_from_token(const char* token, node_t* parent)
{
    static size_t op_hashes[OPERATORS_COUNT] = {};
    static bool hashes_initialized = false;

    if (!hashes_initialized)
    {
        for (size_t i = 0; i < OPERATORS_COUNT; i++)
        {
            op_hashes[i] = compute_hash(OPERATORS[i].string);
        }
        hashes_initialized = true;
    }

    node_type type = NODE_NUM;
    value_of_tree_element data = {};
    size_t token_hash = compute_hash(token);

    // ищем оператора
    for (size_t i = 0; i < OPERATORS_COUNT; i++)
    {
        if (token_hash == op_hashes[i])
        {
            type = NODE_OP;
            data.op_value = OPERATORS[i].op_type;
            break;
        }
    }

    if (type != NODE_OP)
    {
        if (isdigit(token[0]) || (token[0] == '-' && isdigit(token[1])))
            data.num_value = atof(token);
        else
        {
            type = NODE_VAR;
            data.var_definition.hash = token_hash;
            data.var_definition.name = strdup(token);
        }
    }

    node_t* node = create_node(type, data, NULL, NULL);
    if (node != NULL)
        node -> parent = parent;

    else if (type == NODE_VAR)
        free(data.var_definition.name);

    return node;
}






