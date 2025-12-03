#include <math.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include "assert.h"

#include "dump.h"
#include "tree_base.h"
#include "operations.h"
#include "latex_dump.h"
#include "tree_common.h"
#include "variable_parse.h"
#include "logic_functions.h"
#include "tree_error_types.h"





// ==================== DSL ДЛЯ СОЗДАНИЯ УЗЛОВ ====================
#define CREATE_NUM(value)          create_node(NODE_NUM, (value_of_tree_element){.num_value = (value)}, NULL, NULL)
#define CREATE_OP(op, left, right) create_node(NODE_OP,  (value_of_tree_element){.op_value = (op)}, (left), (right))
#define CREATE_UNARY_OP(op, right) create_node(NODE_OP,  (value_of_tree_element){.op_value = (op)}, NULL, (right))
#define CREATE_VAR(name)           create_node(NODE_VAR, (value_of_tree_element){.var_definition = {.name = strdup(name), .hash = compute_hash(name)}}, NULL, NULL)

#define CHECK_AND_CREATE(condition, creator) \
    ((condition) ? (creator) : (NULL))

#define RELEASE_IF_NULL(ptr, ...) \
    do { \
        if (!(ptr)) \
        {           \
            node_t* nodes[] = {__VA_ARGS__}; \
            for (size_t i = 0; i < sizeof(nodes)/sizeof(nodes[0]); i++) \
                if (nodes[i]) free_subtree(nodes[i]); \
        } \
    } while(0)

// ==================== ПРОТОТИПЫ ФУНКЦИЙ ====================
static node_t* differentiate_add_sub(node_t* node, const char* variable_name, operation_type op);
static node_t* differentiate_mul   (node_t* node, const char* variable_name);
static node_t* differentiate_div   (node_t* node, const char* variable_name);
static node_t* differentiate_sin   (node_t* node, const char* variable_name);
static node_t* differentiate_cos   (node_t* node, const char* variable_name);
static node_t* differentiate_pow   (node_t* node, const char* variable_name);
static node_t* differentiate_ln    (node_t* node, const char* variable_name);
static node_t* differentiate_exp   (node_t* node, const char* variable_name);
static node_t* differentiate_power_var_const(node_t* u, node_t* v, node_t* du_dx, node_t* dv_dx);
static node_t* differentiate_power_const_var(node_t* u, node_t* v, node_t* du_dx, node_t* dv_dx);
static node_t* differentiate_power_var_var  (node_t* u, node_t* v, node_t* du_dx, node_t* dv_dx);
static node_t* differentiate_node(node_t* node, const char* variable_name);
static void  free_nodes(int count, ...);
static bool  contains_variable(node_t* node, const char* variable_name);
static void  replace_node(node_t** node_ptr, node_t* new_node);


// ==================== ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ====================


void free_subtree(node_t* node)
{
    if (node == NULL)
        return;

    free_subtree(node -> left);
    free_subtree(node -> right);

    if (node -> type == NODE_VAR && node -> data.var_definition.name != NULL)
        free(node -> data.var_definition.name);

    free(node);
}


static void free_nodes(int count, ...)
{
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; i++)
    {
        node_t* node = va_arg(args, node_t*);
        if (node) free_subtree(node);
    }
    va_end(args);
}


static node_t* create_checked_op(operation_type op, node_t* left, node_t* right)
{
    node_t* result = CREATE_OP(op, left, right);
    RELEASE_IF_NULL(result, left, right);

    return result;
}


static node_t* create_checked_unary_op(operation_type op, node_t* right)
{
    node_t* result = CREATE_UNARY_OP(op, right);
    RELEASE_IF_NULL(result, right);

    return result;
}


static tree_error_type evaluate_tree_recursive(node_t* node, variable_table* var_table, double* result, int depth)
{
    if (node == NULL)
        return TREE_ERROR_NULL_PTR;

    if (result == NULL)
        return TREE_ERROR_NULL_PTR;

    switch (node -> type)
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

                if (node -> data.op_value != OP_SIN && node -> data.op_value != OP_COS &&
                    node -> data.op_value != OP_LN  && node -> data.op_value != OP_EXP)
                {
                    if (node -> left == NULL)
                        return TREE_ERROR_NULL_PTR;

                    error = evaluate_tree_recursive(node -> left, var_table, &left_result, depth + 1);
                    if (error != TREE_ERROR_NO)
                        return error;
                }

                error = evaluate_tree_recursive(node -> right, var_table, &right_result, depth + 1);
                if (error != TREE_ERROR_NO)
                    return error;

                switch (node -> data.op_value)
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
                        if (is_zero(right_result))
                            return TREE_ERROR_DIVISION_BY_ZERO;
                        *result = left_result / right_result;
                        break;
                    case OP_SIN:
                        *result = sin(right_result);
                        break;
                    case OP_COS:
                        *result = cos(right_result);
                        break;
                    case OP_POW:
                        *result = pow(left_result, right_result);
                        break;
                    case OP_LN:
                        if (right_result <= 0)
                            return TREE_ERROR_YCHI_MATAN;
                        *result = log(right_result);
                        break;
                    case OP_EXP:
                        *result = exp(right_result);
                        break;
                    default:
                        return TREE_ERROR_UNKNOWN_OPERATION;
                }
                return TREE_ERROR_NO;
            }

        case NODE_VAR:
            {
                if (node -> data.var_definition.name == NULL)
                    return TREE_ERROR_VARIABLE_NOT_FOUND;

                char* var_name = node -> data.var_definition.name;
                double value = 0.0;
                tree_error_type error = get_variable_value(var_table, var_name, &value);

                if (error == TREE_ERROR_VARIABLE_NOT_FOUND)
                {
                    error = add_variable(var_table, var_name);
                    if (error != TREE_ERROR_NO)
                        return error;

                    error = request_variable_value(var_table, var_name);
                    if (error != TREE_ERROR_NO) return error;

                    error = get_variable_value(var_table, var_name, &value);
                    if (error != TREE_ERROR_NO) return error;
                }
                else if (error == TREE_ERROR_VARIABLE_UNDEFINED)
                {
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
    if (tree == NULL || var_table == NULL || result == NULL)
        return TREE_ERROR_NULL_PTR;

    if (tree -> root == NULL)
        return TREE_ERROR_NULL_PTR;

    return evaluate_tree_recursive(tree -> root, var_table, result, 0);
}


node_t* create_node(node_type type, value_of_tree_element data, node_t* left, node_t* right)
{
    node_t* node = (node_t*)calloc(1, sizeof(node_t));
    if (!node)
        return NULL;

    node -> type = type;
    node -> left = left;
    node -> right = right;
    node -> parent = NULL;

     node -> data = data;

    if (type == NODE_VAR && data.var_definition.name != NULL)
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


static node_t* copy_node(node_t* original)
{
    if (original == NULL)
        return NULL;

    value_of_tree_element data = {};

    switch (original -> type)
    {
        case NODE_NUM:
            data.num_value = original -> data.num_value;
            return CREATE_NUM(data.num_value);

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
            if (original -> data.op_value == OP_SIN || original -> data.op_value == OP_COS ||
                original -> data.op_value == OP_LN  || original -> data.op_value == OP_EXP)
            {
                return CREATE_UNARY_OP(data.op_value, copy_node(original -> right));
            }
            else
            {
                return CREATE_OP(data.op_value, copy_node(original -> left), copy_node(original -> right));
            }
        default:
            return NULL;
    }
}


static bool contains_variable(node_t* node, const char* variable_name)
{
    if (node == NULL)
        return false;

    unsigned int target_hash = compute_hash(variable_name);

    switch (node -> type)
    {
        case NODE_VAR:
            return (node -> data.var_definition.hash == target_hash) &&
                   (node -> data.var_definition.name != NULL)        &&
                   (strcmp(node -> data.var_definition.name, variable_name) == 0);

        case NODE_OP:
            return contains_variable(node -> left, variable_name) ||
                   contains_variable(node -> right, variable_name);

        case NODE_NUM:
        default:
            return false;
    }
}


// ==================== ФУНКЦИИ ДИФФЕРЕНЦИРОВАНИЯ ====================


static node_t* differentiate_add_sub(node_t* node, const char* variable_name, operation_type op)
{
    node_t* left_deriv  = differentiate_node(node -> left,  variable_name);
    node_t* right_deriv = differentiate_node(node -> right, variable_name);

    if (!left_deriv || !right_deriv)
    {
        free_nodes(2, left_deriv, right_deriv);
        return NULL;
    }

    return create_checked_op(op, left_deriv, right_deriv);
}


static node_t* differentiate_mul(node_t* node, const char* variable_name)
{
    node_t* u = copy_node(node -> left);
    node_t* v = copy_node(node -> right);
    node_t* du_dx = differentiate_node(node -> left,  variable_name);
    node_t* dv_dx = differentiate_node(node -> right, variable_name);

    if (!u || !v || !du_dx || !dv_dx)
    {
        free_nodes(4, u, v, du_dx, dv_dx);
        return NULL;
    }

    node_t* term1 = create_checked_op(OP_MUL, u, dv_dx);
    if (!term1)
    {
        free_nodes(3, v, du_dx, dv_dx);
        return NULL;
    }

    node_t* term2 = create_checked_op(OP_MUL, v, du_dx);
    if (!term2)
    {
        free_nodes(2, term1, du_dx);
        return NULL;
    }

    node_t* result = create_checked_op(OP_ADD, term1, term2);
    if (!result)
    {
        free_nodes(2, term1, term2);
    }

    return result;
}


static node_t* differentiate_div(node_t* node, const char* variable_name)
{
    node_t* u = copy_node(node -> left);
    node_t* v = copy_node(node -> right);
    node_t* du_dx = differentiate_node(node -> left, variable_name);
    node_t* dv_dx = differentiate_node(node -> right, variable_name);

    if (!u || !v || !du_dx || !dv_dx)
    {
        free_nodes(4, u, v, du_dx, dv_dx);
        return NULL;
    }

    node_t* numerator_term1 = create_checked_op(OP_MUL, v, du_dx);
    if (!numerator_term1)
    {
        free_nodes(4, u, v, du_dx, dv_dx);
        return NULL;
    }

    node_t* numerator_term2 = create_checked_op(OP_MUL, u, dv_dx);
    if (!numerator_term2)
    {
        free_nodes(3, numerator_term1, u, dv_dx);
        return NULL;
    }

    node_t* numerator = create_checked_op(OP_SUB, numerator_term1, numerator_term2);
    if (!numerator)
    {
        free_nodes(2, numerator_term1, numerator_term2);
        return NULL;
    }

    node_t* v_copy1 = copy_node(node -> right);
    node_t* v_copy2 = copy_node(node -> right);
    node_t* v_squared = create_checked_op(OP_MUL, v_copy1, v_copy2);
    if (!v_squared)
    {
        free_nodes(3, numerator, v_copy1, v_copy2);
        return NULL;
    }

    node_t* result = create_checked_op(OP_DIV, numerator, v_squared);
    if (!result)
        free_nodes(2, numerator, v_squared);

    return result;
}


static node_t* differentiate_sin(node_t* node, const char* variable_name)
{
    node_t* u = copy_node(node -> right);
    node_t* du_dx = differentiate_node(node -> right, variable_name);

    if (!u || !du_dx)
    {
        free_nodes(2, u, du_dx);
        return NULL;
    }

    node_t* cos_u = create_checked_unary_op(OP_COS, u);
    if (!cos_u)
    {
        free_nodes(1, du_dx);
        return NULL;
    }

    node_t* result = create_checked_op(OP_MUL, cos_u, du_dx);
    if (!result)
    {
        free_nodes(2, cos_u, du_dx);
    }

    return result;
}


static node_t* differentiate_cos(node_t* node, const char* variable_name)
{
    node_t* u = copy_node(node -> right);
    node_t* du_dx = differentiate_node(node -> right, variable_name);

    if (!u || !du_dx)
    {
        free_nodes(2, u, du_dx);
        return NULL;
    }

    node_t* sin_u = create_checked_unary_op(OP_SIN, u);
    if (!sin_u)
    {
        free_nodes(1, du_dx);
        return NULL;
    }

    node_t* minus_one = CREATE_NUM(-1.0);
    if (!minus_one)
    {
        free_nodes(2, sin_u, du_dx);
        return NULL;
    }

    node_t* minus_sin_u = create_checked_op(OP_MUL, minus_one, sin_u);
    if (!minus_sin_u)
    {
        free_nodes(3, minus_one, sin_u, du_dx);
        return NULL;
    }

    node_t* result = create_checked_op(OP_MUL, minus_sin_u, du_dx);
    if (!result)
        free_nodes(2, minus_sin_u, du_dx);

    return result;
}


static node_t* differentiate_power_var_const(node_t* u, node_t* v, node_t* du_dx, node_t* dv_dx)
{
    node_t* a_minus_one = CREATE_NUM(v -> data.num_value - 1.0);
    if (!a_minus_one)
        return NULL;

    node_t* u_pow_a_minus_one = create_checked_op(OP_POW, u, a_minus_one);
    if (!u_pow_a_minus_one)
    {
        free_nodes(1, a_minus_one);
        return NULL;
    }

    node_t* a_times_pow = create_checked_op(OP_MUL, v, u_pow_a_minus_one);
    if (!a_times_pow)
    {
        free_nodes(1, u_pow_a_minus_one);
        return NULL;
    }

    node_t* result = create_checked_op(OP_MUL, a_times_pow, du_dx);
    if (!result)
        free_nodes(1, a_times_pow);
    else
        free_subtree(dv_dx);

    return result;
}


static node_t* differentiate_power_const_var(node_t* u, node_t* v, node_t* du_dx, node_t* dv_dx)
{
    node_t* a_pow_x = create_checked_op(OP_POW, u, v);
    if (!a_pow_x)
        return NULL;

    node_t* u_copy = copy_node(u);
    if (!u_copy)
    {
        free_nodes(1, a_pow_x);
        return NULL;
    }

    node_t* ln_a = create_checked_unary_op(OP_LN, u_copy);
    if (!ln_a)
    {
        free_nodes(2, a_pow_x, u_copy);
        return NULL;
    }

    node_t* result = create_checked_op(OP_MUL, a_pow_x, ln_a);
    if (!result)
        free_nodes(2, a_pow_x, ln_a);
    else
        free_nodes(2, du_dx, dv_dx);

    return result;
}


static node_t* differentiate_power_var_var(node_t* u, node_t* v, node_t* du_dx, node_t* dv_dx)
{
    node_t* u_pow_v = create_checked_op(OP_POW, u, v);
    if (!u_pow_v)
        return NULL;

    node_t* u_copy_for_ln  = copy_node(u);
    node_t* u_copy_for_div = copy_node(u);
    node_t* v_copy_for_div = copy_node(v);

    if (!u_copy_for_ln || !u_copy_for_div || !v_copy_for_div)
    {
        free_nodes(4, u_pow_v, u_copy_for_ln, u_copy_for_div, v_copy_for_div);
        return NULL;
    }

    node_t* ln_u = create_checked_unary_op(OP_LN, u_copy_for_ln);
    if (!ln_u)
    {
        free_nodes(4, u_pow_v, u_copy_for_ln, u_copy_for_div, v_copy_for_div);
        return NULL;
    }

    node_t* dv_ln_u = create_checked_op(OP_MUL, dv_dx, ln_u);
    if (!dv_ln_u)
    {
        free_nodes(4, u_pow_v, u_copy_for_div, v_copy_for_div, ln_u);
        return NULL;
    }

    node_t* v_div_u = create_checked_op(OP_DIV, v_copy_for_div, u_copy_for_div);
    if (!v_div_u)
    {
        free_nodes(3, u_pow_v, dv_ln_u, v_copy_for_div);
        return NULL;
    }

    node_t* v_du_div_u = create_checked_op(OP_MUL, v_div_u, du_dx);
    if (!v_du_div_u)
    {
        free_nodes(3, u_pow_v, dv_ln_u, v_div_u);
        return NULL;
    }

    node_t* bracket = create_checked_op(OP_ADD, dv_ln_u, v_du_div_u);
    if (!bracket)
    {
        free_nodes(3, u_pow_v, dv_ln_u, v_du_div_u);
        return NULL;
    }

    node_t* result = create_checked_op(OP_MUL, u_pow_v, bracket);
    if (!result)
        free_nodes(2, u_pow_v, bracket);

    return result;
}


static node_t* differentiate_pow(node_t* node, const char* variable_name)
{
    node_t* u = copy_node(node -> left);
    node_t* v = copy_node(node -> right);
    node_t* du_dx = differentiate_node(node -> left, variable_name);
    node_t* dv_dx = differentiate_node(node -> right, variable_name);

    if (!u || !v || !du_dx || !dv_dx)
    {
        free_nodes(4, u, v, du_dx, dv_dx);
        return NULL;
    }

    bool left_has_var  = contains_variable(node -> left, variable_name);
    bool right_has_var = contains_variable(node -> right, variable_name);

    node_t* result = NULL;

    if (left_has_var && !right_has_var)
        result = differentiate_power_var_const(u, v, du_dx, dv_dx);
    else if (!left_has_var && right_has_var)
        result = differentiate_power_const_var(u, v, du_dx, dv_dx);
    else
        result = differentiate_power_var_var(u, v, du_dx, dv_dx);

    if (!result)
        free_nodes(4, u, v, du_dx, dv_dx);

    return result;
}


static node_t* differentiate_ln(node_t* node, const char* variable_name)
{
    node_t* u = copy_node(node -> right);
    node_t* du_dx = differentiate_node(node -> right, variable_name);

    if (!u || !du_dx)
    {
        free_nodes(2, u, du_dx);
        return NULL;
    }

    node_t* one = CREATE_NUM(1.0);
    if (!one)
    {
        free_nodes(2, u, du_dx);
        return NULL;
    }

    node_t* one_div_u = create_checked_op(OP_DIV, one, u);
    if (!one_div_u)
    {
        free_nodes(3, one, u, du_dx);
        return NULL;
    }

    node_t* result = create_checked_op(OP_MUL, one_div_u, du_dx);
    if (!result)
        free_nodes(2, one_div_u, du_dx);

    return result;
}


static node_t* differentiate_exp(node_t* node, const char* variable_name)
{
    node_t* u = copy_node(node -> right);
    node_t* du_dx = differentiate_node(node -> right, variable_name);

    if (!u || !du_dx)
    {
        free_nodes(2, u, du_dx);
        return NULL;
    }

    node_t* exp_u = create_checked_unary_op(OP_EXP, u);
    if (!exp_u)
    {
        free_nodes(1, du_dx);
        return NULL;
    }

    node_t* result = create_checked_op(OP_MUL, exp_u, du_dx);
    if (!result)
        free_nodes(2, exp_u, du_dx);

    return result;
}


static node_t* differentiate_node(node_t* node, const char* variable_name)
{
    if (node == NULL)
        return NULL;

    switch (node -> type)
    {
        case NODE_NUM:
            return CREATE_NUM(0.0);

        case NODE_VAR:
            if (node -> data.var_definition.name &&
                strcmp(node -> data.var_definition.name, variable_name) == 0)
            {
                return CREATE_NUM(1.0);
            }
            else
            {
                return CREATE_NUM(0.0);
            }

        case NODE_OP:
            switch (node -> data.op_value)
            {
                case OP_ADD:
                case OP_SUB:
                    return differentiate_add_sub(node, variable_name, node -> data.op_value);

                case OP_MUL:
                    return differentiate_mul(node, variable_name);

                case OP_DIV:
                    return differentiate_div(node, variable_name);

                case OP_SIN:
                    return differentiate_sin(node, variable_name);

                case OP_COS:
                    return differentiate_cos(node, variable_name);

                case OP_POW:
                    return differentiate_pow(node, variable_name);

                case OP_LN:
                    return differentiate_ln(node, variable_name);

                case OP_EXP:
                    return differentiate_exp(node, variable_name);

                default:
                    return CREATE_NUM(0.0);
            }

        default:
            return CREATE_NUM(0.0);
    }
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
    static operation_info operations[] = {
        {0, "+",   OP_ADD},
        {0, "-",   OP_SUB},
        {0, "*",   OP_MUL},
        {0, "/",   OP_DIV},
        {0, "sin", OP_SIN},
        {0, "cos", OP_COS},
        {0, "^",   OP_POW},
        {0, "ln",  OP_LN},
        {0, "exp", OP_EXP}
    };

    static size_t operations_count = sizeof(operations) / sizeof(operations[0]);
    static bool hashes_initialized = false;

    if (!hashes_initialized)
    {
        for (size_t i = 0; i < operations_count; i++)
        {
            operations[i].hash = compute_hash(operations[i].name);
        }
        hashes_initialized = true;
    }

    unsigned int token_hash = compute_hash(token);
    node_t* node = NULL;

    for (size_t i = 0; i < operations_count; i++)
    {
        if (token_hash == operations[i].hash)
        {
            node = CREATE_OP(operations[i].op_value, NULL, NULL);
            break;
        }
    }

    if (node == NULL)
    {
        if (isdigit(token[0]) || (token[0] == '-' && isdigit(token[1])))
        {
            node = CREATE_NUM(atof(token));
        }
        else
        {
            value_of_tree_element data = {};
            data.var_definition.hash = token_hash;
            data.var_definition.name = strdup(token);
            node = create_node(NODE_VAR, data, NULL, NULL);
        }
    }

    if (node != NULL)
    {
        node -> parent = parent;
    }

    return node;
}


static void replace_node(node_t** node_ptr, node_t* new_node)
{
    if (node_ptr == NULL || *node_ptr == NULL)
        return;

    node_t* old_node = *node_ptr;
    *node_ptr = new_node;

    if (new_node != NULL)
        new_node -> parent = old_node -> parent;

    free_subtree(old_node);
}


// ==================== ФУНКЦИИ ОПТИМИЗАЦИИ С ДАМПОМ ====================


static tree_error_type constant_folding_optimization_with_dump(node_t** node, FILE* tex_file, tree_t* tree, variable_table* var_table)
{
    if (node == NULL || *node == NULL)
        return TREE_ERROR_NULL_PTR;

    tree_error_type error = TREE_ERROR_NO;

    if ((*node) -> left != NULL)
    {
        error = constant_folding_optimization_with_dump(&(*node) -> left, tex_file, tree, var_table);
        if (error != TREE_ERROR_NO)
            return error;
    }

    if ((*node) -> right != NULL)
    {
        error = constant_folding_optimization_with_dump(&(*node) -> right, tex_file, tree, var_table);
        if (error != TREE_ERROR_NO)
            return error;
    }

    if ((*node) -> type == NODE_OP)
    {
        if (((*node) -> data.op_value == OP_SIN || (*node) -> data.op_value == OP_COS ||
             (*node) -> data.op_value == OP_LN  || (*node) -> data.op_value == OP_EXP) &&
             (*node) -> right != NULL && (*node) -> right -> type == NODE_NUM)
        {
            double result = 0.0;
            bool can_fold = true;

            switch ((*node) -> data.op_value)
            {
                case OP_SIN:
                    result = sin((*node) -> right -> data.num_value);
                    break;
                case OP_COS:
                    result = cos((*node) -> right -> data.num_value);
                    break;
                case OP_LN:
                    if ((*node) -> right -> data.num_value <= 0)
                        can_fold = false;
                    else
                        result = log((*node) -> right -> data.num_value);
                    break;
                case OP_EXP:
                    result = exp((*node) -> right -> data.num_value);
                    break;
                default:
                    can_fold = false;
                    break;
            }

            if (can_fold)
            {
                node_t* new_node = CREATE_NUM(result);
                if (new_node != NULL)
                {
                    replace_node(node, new_node);

                    double new_result = 0.0;
                    if (evaluate_tree(tree, var_table, &new_result) == TREE_ERROR_NO && tex_file != NULL)
                    {
                        char description[MAX_TEX_DESCRIPTION_LENGTH] = {0};
                        snprintf(description, sizeof(description),
                                "constant folding simplified part of expression to: %.2f", result);
                        dump_optimization_step_to_file(tex_file, description, tree, new_result);
                    }
                }
            }
        }
        else if ((*node) -> left  != NULL && (*node) -> left -> type  == NODE_NUM &&
                 (*node) -> right != NULL && (*node) -> right -> type == NODE_NUM)
        {
            double left_val  = (*node) -> left -> data.num_value;
            double right_val = (*node) -> right -> data.num_value;
            double result = 0.0;
            bool can_fold = true;

            switch ((*node) -> data.op_value)
            {
                case OP_ADD:
                    result = left_val + right_val;
                    break;
                case OP_SUB:
                    result = left_val - right_val;
                    break;
                case OP_MUL:
                    result = left_val * right_val;
                    break;
                case OP_DIV:
                    if (is_zero(right_val))
                        can_fold = false;
                    else
                        result = left_val / right_val;
                    break;
                default:
                    can_fold = false;
                    break;
            }

            if (can_fold)
            {
                node_t* new_node = CREATE_NUM(result);
                if (new_node != NULL)
                {
                    replace_node(node, new_node);

                    double new_result = 0.0;
                    if (evaluate_tree(tree, var_table, &new_result) == TREE_ERROR_NO && tex_file != NULL)
                    {
                        char description[MAX_TEX_DESCRIPTION_LENGTH] = {0};
                        snprintf(description, sizeof(description),
                                "constant folding simplified part of expression to: %.2f", result);
                        dump_optimization_step_to_file(tex_file, description, tree, new_result);
                    }
                }
            }
        }
    }

    return TREE_ERROR_NO;
}


static tree_error_type neutral_elements_optimization_with_dump(node_t** node, FILE* tex_file, tree_t* tree, variable_table* var_table)
{
    if (node == NULL || *node == NULL)
        return TREE_ERROR_NULL_PTR;

    tree_error_type error = TREE_ERROR_NO;

    if ((*node) -> left != NULL)
    {
        error = neutral_elements_optimization_with_dump(&(*node) -> left, tex_file, tree, var_table);
        if (error != TREE_ERROR_NO)
            return error;
    }

    if ((*node) -> right != NULL)
    {
        error = neutral_elements_optimization_with_dump(&(*node) -> right, tex_file, tree, var_table);
        if (error != TREE_ERROR_NO)
            return error;
    }

    if ((*node) -> type == NODE_OP)
    {
        node_t* new_node = NULL;
        const char* description = NULL;

        switch ((*node) -> data.op_value)
        {
            case OP_ADD:
                if ((*node) -> right != NULL && (*node) -> right -> type == NODE_NUM &&
                    is_zero((*node) -> right -> data.num_value))
                {
                    new_node = copy_node((*node) -> left);
                    description = "adding zero simplified";
                }
                else if ((*node) -> left != NULL && (*node) -> left -> type == NODE_NUM &&
                         is_zero((*node) -> left -> data.num_value))
                {
                    new_node = copy_node((*node) -> right);
                    description = "adding zero simplified";
                }
                break;

            case OP_SUB:
                if ((*node) -> right != NULL && (*node) -> right -> type == NODE_NUM &&
                    is_zero((*node) -> right -> data.num_value))
                {
                    new_node = copy_node((*node) -> left);
                    description = "- 0 simplified";
                }
                break;

            case OP_MUL:
                if (((*node) -> left != NULL && (*node) -> left -> type == NODE_NUM &&
                     is_zero((*node) -> left -> data.num_value)) ||
                    ((*node) -> right != NULL && (*node) -> right -> type == NODE_NUM &&
                     is_zero((*node) -> right -> data.num_value)))
                {
                    new_node = CREATE_NUM(0.0);
                    description = "mul zero simplified";
                }
                else if ((*node) -> right != NULL && (*node) -> right -> type == NODE_NUM &&
                         is_one((*node) -> right -> data.num_value))
                {
                    new_node = copy_node((*node) -> left);
                    description = "mul one simplified";
                }
                else if ((*node) -> left != NULL && (*node) -> left -> type == NODE_NUM &&
                         is_one((*node) -> left -> data.num_value))
                {
                    new_node = copy_node((*node) -> right);
                    description = "mul one simplified";
                }
                break;

            case OP_DIV:
                if ((*node) -> left != NULL && (*node) -> left -> type == NODE_NUM &&
                    is_zero((*node) -> left -> data.num_value) &&
                    (*node) -> right != NULL &&
                    !((*node) -> right -> type == NODE_NUM && is_zero((*node) -> right -> data.num_value)))
                {
                    new_node = CREATE_NUM(0.0);
                    description = "0 / simplified";
                }
                else if ((*node) -> right != NULL && (*node) -> right -> type == NODE_NUM &&
                         is_one((*node) -> right -> data.num_value))
                {
                    new_node = copy_node((*node) -> left);
                    description = " / 1 simplified";
                }
                break;
            case OP_POW:
                if ((*node) -> right != NULL && (*node) -> right -> type == NODE_NUM &&
                    is_zero((*node) -> right -> data.num_value))
                {
                    new_node = CREATE_NUM(1.0);
                    description = "^0 simplified";
                }
                else if ((*node) -> right != NULL && (*node) -> right -> type == NODE_NUM &&
                         is_one((*node) -> right -> data.num_value))
                {
                    new_node = copy_node((*node) -> left);
                    description = "^1 simplified";
                }
                else if ((*node) -> left != NULL && (*node) -> left -> type == NODE_NUM &&
                         is_one((*node) -> left -> data.num_value))
                {
                    new_node = CREATE_NUM(1.0);
                    description = "1^ simplified";
                }
                break;
            default:
                break;
        }

        if (new_node != NULL && description != NULL)
        {
            replace_node(node, new_node);

            double new_result = 0.0;
            if (evaluate_tree(tree, var_table, &new_result) == TREE_ERROR_NO && tex_file != NULL)
                dump_optimization_step_to_file(tex_file, description, tree, new_result);
        }
    }

    return TREE_ERROR_NO;
}


size_t count_tree_nodes(node_t* node)
{
    if (node == NULL)
        return 0;

    return 1 + count_tree_nodes(node -> left) + count_tree_nodes(node -> right);
}


static tree_error_type optimize_subtree_with_dump(node_t** node, FILE* tex_file, tree_t* tree, variable_table* var_table)
{
    if (node == NULL || *node == NULL)
        return TREE_ERROR_NULL_PTR;

    size_t old_size = 0;
    size_t new_size = count_tree_nodes(*node);
    tree_error_type error = TREE_ERROR_NO;

    do
    {
        old_size = new_size;

        error = constant_folding_optimization_with_dump(node, tex_file, tree, var_table);
        if (error != TREE_ERROR_NO) return error;

        error = neutral_elements_optimization_with_dump(node, tex_file, tree, var_table);
        if (error != TREE_ERROR_NO) return error;

        new_size = count_tree_nodes(*node);
    } while (new_size != old_size);

    return TREE_ERROR_NO;
}


tree_error_type optimize_tree_with_dump(tree_t* tree, FILE* tex_file, variable_table* var_table)
{
    if (tree == NULL)
        return TREE_ERROR_NULL_PTR;

    if (tree -> root == NULL)
        return TREE_ERROR_NULL_PTR;

    double result_before = 0.0;
    evaluate_tree(tree, var_table, &result_before);

    if (tex_file != NULL)
    {
        fprintf(tex_file, "\\section*{Optimization}\n");
        fprintf(tex_file, "Before optimization: ");

        char expression[MAX_LENGTH_OF_TEX_EXPRESSION] = {};
        int position = 0;
        tree_to_string_simple(tree -> root, expression, &position, sizeof(expression));
        fprintf(tex_file, "\\[ %s \\]\n\n", expression);
        // fprintf(tex_file, "Result before optimization: \\[ %.6f \\]\n\n", result_before);
    }

    tree_error_type error = optimize_subtree_with_dump(&tree -> root, tex_file, tree, var_table);
    if (error != TREE_ERROR_NO)
        return error;

    tree -> size = count_tree_nodes(tree -> root);

    double result_after = 0.0;
    evaluate_tree(tree, var_table, &result_after);

    if (tex_file != NULL)
    {
        fprintf(tex_file, "\\subsection*{Result optimization}\n");

        char expression[MAX_LENGTH_OF_TEX_EXPRESSION] = {};
        int position = 0;
        tree_to_string_simple(tree -> root, expression, &position, sizeof(expression));
        fprintf(tex_file, "Final expression: \\[ %s \\]\n\n", expression);
        fprintf(tex_file, "Final result: \\[ %.6f \\]\n\n", result_after);
    }

    return TREE_ERROR_NO;
}


// ==================== UNDEF MACROS ====================
#undef CREATE_NUM
#undef CREATE_OP
#undef CREATE_UNARY_OP
#undef CREATE_VAR
#undef CHECK_AND_CREATE
#undef RELEASE_IF_NULL
