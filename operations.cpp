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

    printf("DEBUG evaluate_tree_recursive: node type = %d\n", node->type);

    switch(node -> type)
    {
        case NODE_NUM:
            *result = node -> data.num_value;
            printf("DEBUG evaluate_tree_recursive: NODE_NUM, value = %f\n", *result);
            return TREE_ERROR_NO;

        case NODE_OP:
            {
                double left_result = 0, right_result = 0;
                tree_error_type error = TREE_ERROR_NO;

                printf("DEBUG evaluate_tree_recursive: NODE_OP, op_value = %d\n", node->data.op_value);

                // для всех операций проверяем правый аргумент
                if (node -> right == NULL)
                {
                    printf("DEBUG evaluate_tree_recursive: right node is NULL\n");
                    return TREE_ERROR_NULL_PTR;
                }

                // для бинарных операций вычисляем оба аргумента
                if (node -> data.op_value != OP_SIN && node -> data.op_value != OP_COS)
                {
                    if (node -> left == NULL)
                    {
                        printf("DEBUG evaluate_tree_recursive: left node is NULL for binary op\n");
                        return TREE_ERROR_NULL_PTR;
                    }

                    printf("DEBUG evaluate_tree_recursive: evaluating left node\n");
                    error = evaluate_tree_recursive(node -> left, var_table, &left_result);
                    if (error != TREE_ERROR_NO)
                    {
                        printf("DEBUG evaluate_tree_recursive: left evaluation failed with error %d\n", error);
                        return error;
                    }
                    printf("DEBUG evaluate_tree_recursive: left result = %f\n", left_result);
                }

                printf("DEBUG evaluate_tree_recursive: evaluating right node\n");
                error = evaluate_tree_recursive(node -> right, var_table, &right_result);
                if (error != TREE_ERROR_NO)
                {
                    printf("DEBUG evaluate_tree_recursive: right evaluation failed with error %d\n", error);
                    return error;
                }
                printf("DEBUG evaluate_tree_recursive: right result = %f\n", right_result);

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
                        {
                            printf("DEBUG evaluate_tree_recursive: division by zero\n");
                            return TREE_ERROR_DIVISION_BY_ZERO;
                        }
                        *result = left_result / right_result;
                        break;
                    case OP_SIN:
                        *result = sin(right_result);
                        break;
                    case OP_COS:
                        *result = cos(right_result);
                        break;
                    default:
                        printf("DEBUG evaluate_tree_recursive: unknown operation %d\n", node->data.op_value);
                        return TREE_ERROR_UNKNOWN_OPERATION;
                }
                printf("DEBUG evaluate_tree_recursive: operation result = %f\n", *result);
                return TREE_ERROR_NO;
            }

        case NODE_VAR:
            {
                // Используем сохраненное имя переменной, а не формируем из хеша
                const char* var_name = node->data.var_definition.name;
                printf("DEBUG evaluate_tree_recursive: NODE_VAR, name = '%s'\n", var_name);

                double value = 0.0;
                tree_error_type error = get_variable_value(var_table, var_name, &value);

                if (error == TREE_ERROR_VARIABLE_NOT_FOUND)
                {
                    printf("DEBUG evaluate_tree_recursive: variable not found, adding to table\n");
                    error = add_variable(var_table, var_name);
                    if (error != TREE_ERROR_NO)
                    {
                        printf("DEBUG evaluate_tree_recursive: add_variable failed with error %d\n", error);
                        return error;
                    }

                    printf("Variable '%s' not defined.\n", var_name);
                    printf("DEBUG evaluate_tree_recursive: calling request_variable_value\n");
                    error = request_variable_value(var_table, var_name);
                    printf("DEBUG evaluate_tree_recursive: returned from request_variable_value with error %d\n", error);
                    if (error != TREE_ERROR_NO)
                    {
                        printf("DEBUG evaluate_tree_recursive: request_variable_value failed with error %d\n", error);
                        return error;
                    }

                    printf("DEBUG evaluate_tree_recursive: calling get_variable_value again\n");
                    error = get_variable_value(var_table, var_name, &value);
                    printf("DEBUG evaluate_tree_recursive: second get_variable_value returned error %d, value = %f\n", error, value);
                    if (error != TREE_ERROR_NO)
                    {
                        printf("DEBUG evaluate_tree_recursive: second get_variable_value failed with error %d\n", error);
                        return error;
                    }
                }
                else if (error == TREE_ERROR_VARIABLE_UNDEFINED)
                {
                    printf("DEBUG evaluate_tree_recursive: variable undefined\n");
                    printf("Variable '%s' not defined.\n", var_name);
                    printf("DEBUG evaluate_tree_recursive: calling request_variable_value\n");
                    error = request_variable_value(var_table, var_name);
                    printf("DEBUG evaluate_tree_recursive: returned from request_variable_value with error %d\n", error);
                    if (error != TREE_ERROR_NO)
                    {
                        printf("DEBUG evaluate_tree_recursive: request_variable_value failed with error %d\n", error);
                        return error;
                    }

                    printf("DEBUG evaluate_tree_recursive: calling get_variable_value again\n");
                    error = get_variable_value(var_table, var_name, &value);
                    printf("DEBUG evaluate_tree_recursive: second get_variable_value returned error %d, value = %f\n", error, value);
                    if (error != TREE_ERROR_NO)
                    {
                        printf("DEBUG evaluate_tree_recursive: second get_variable_value failed with error %d\n", error);
                        return error;
                    }
                }
                else if (error != TREE_ERROR_NO)
                {
                    printf("DEBUG evaluate_tree_recursive: get_variable_value failed with error %d\n", error);
                    return error;
                }

                *result = value;
                printf("DEBUG evaluate_tree_recursive: final variable value = %f\n", *result);
                return TREE_ERROR_NO;
            }
        default:
            printf("DEBUG evaluate_tree_recursive: unknown node type %d\n", node->type);
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


