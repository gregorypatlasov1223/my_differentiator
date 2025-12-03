#include <assert.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "tree_base.h"
#include "new_input.h"


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
// ===================================================================

static parser_context* create_parser_context(variable_table* var_table)
{
    static operation_info default_operations[] = {
        {0, "sin", OP_SIN},
        {0, "cos", OP_COS},
        {0, "ln", OP_LN},
        {0, "exp", OP_EXP}
    };

    static size_t default_operations_count = sizeof(default_operations) / sizeof(default_operations[0]);

    parser_context* context = (parser_context*)calloc(1, sizeof(parser_context));
    if (!context)
        return NULL;

    context -> var_table = var_table;

    context -> operations = default_operations; //сохраняем указатель на статический массив зарезервированных операций
    context -> operations_count = default_operations_count;
    context -> hashes_initialized = false;

    return context;
}

static void free_parser_context(parser_context* context)
{
    free(context);
}

static void initialize_operation_hashes(parser_context* context)
{
    if (!context || context->hashes_initialized)
        return;

    for (size_t i = 0; i < context -> operations_count; i++)
    {
        operation_info* op = &context -> operations[i];
        op -> hash = compute_hash(op->name);
    }

    context->hashes_initialized = true;
}

node_t* get_G(const char** string, variable_table* var_table)
{
    assert(string);
    assert(var_table);

    parser_context* context = create_parser_context(var_table);
    if (!context)
        return NULL;

    node_t* val = get_E(string, context);

    if (**string != '$')
    {
        printf("Expected end of expression '$'\n");
        syntax_error();
        if (val)
            free_subtree(val);

        free_parser_context(context);
        return NULL;
    }

    free_parser_context(context);
    return val;
}

node_t* get_E(const char** string, parser_context* context)
{
    assert(string);
    assert(context);

    node_t* val = get_T(string, context);
    if (!val)
        return NULL;

    while (**string == '+' || **string == '-')
    {
        char op_char = **string;
        operation_type op = (op_char == '+') ? OP_ADD : OP_SUB;

        (*string)++;
        node_t* val2 = get_T(string, context);
        if (!val2)
        {
            free_subtree(val);
            return NULL;
        }

        node_t* new_val = CREATE_OP(op, val, val2);
        if (!new_val)
        {
            free_subtree(val);
            free_subtree(val2);
            return NULL;
        }
        val = new_val;
    }

    return val;
}

node_t* get_T(const char** string, parser_context* context)
{
    assert(string);
    assert(context);

    node_t* val = get_F(string, context);
    if (!val)
        return NULL;

    while (**string == '*' || **string == '/')
    {
        char op_char = **string;
        operation_type op = (op_char == '*') ? OP_MUL : OP_DIV;

        (*string)++;
        node_t* val2 = get_F(string, context);
        if (!val2)
        {
            free_subtree(val);
            return NULL;
        }

        node_t* new_val = CREATE_OP(op, val, val2);
        if (!new_val)
        {
            free_subtree(val);
            free_subtree(val2);
            return NULL;
        }
        val = new_val;
    }

    return val;
}

node_t* get_F(const char** string, parser_context* context)
{
    assert(string);
    assert(context);

    node_t* val = get_P(string, context);
    if (!val)
        return NULL;

    while (**string == '^')
    {
        (*string)++;
        node_t* exponent = get_P(string, context);
        if (!exponent)
        {
            free_subtree(val);
            return NULL;
        }

        node_t* new_val = CREATE_OP(OP_POW, val, exponent);
        if (!new_val)
        {
            free_subtree(val);
            free_subtree(exponent);
            return NULL;
        }
        val = new_val;
    }

    return val;
}

node_t* get_P(const char** string, parser_context* context)
{
    assert(string);
    assert(context);

    node_t* func_node = get_function(string, context);
    if (func_node)
        return func_node;

    if (**string == '(')
    {
        (*string)++;
        node_t* val = get_E(string, context);
        if (!val)
            return NULL;

        if (**string != ')')
        {
            printf("Expected closing ')'\n");
            syntax_error();
            free_subtree(val);
            return NULL;
        }
        else
        {
            (*string)++;
        }
        return val;
    }

    node_t* result = get_N(string);
    if (result != NULL) return result;

    result = get_V(string, context);
    if (result != NULL) return result;

    return NULL;
}

node_t* get_N(const char** string)
{
    assert(string);

    if (isdigit(**string))
    {
        int val = 0;
        const char* start = *string;

        while ('0' <= **string && **string <= '9')
        {
            val = **string - '0' + val * 10;
            (*string)++;
        }

        if (start == *string)
        {
            syntax_error();
            return NULL;
        }

        return CREATE_NUM((double)val);
    }

    return NULL;
}

node_t* get_V(const char** string, parser_context* context)
{
    assert(string);
    assert(context);

    if (!isalpha(**string))
        return NULL;

    char var_name[MAX_VARIABLE_LENGTH] = {};
    int i = 0;
    const char* start = *string;

    while (**string >= 'a' && **string <= 'z' && i < 255)
    {
        var_name[i++] = **string;
        (*string)++;
    }

    if (start == *string)
    {
        syntax_error();
        return NULL;
    }

    tree_error_type error = add_variable(context -> var_table, var_name);
    if (error != TREE_ERROR_NO && error != TREE_ERROR_VARIABLE_ALREADY_EXISTS &&
        error != TREE_ERROR_REDEFINITION_VARIABLE)
    {
        printf("Error adding variable to table: %d\n", error);
        return NULL;
    }

    value_of_tree_element data = {};
    data.var_definition.name = strdup(var_name);
    data.var_definition.hash = compute_hash(var_name);

    return create_node(NODE_VAR, data, NULL, NULL);
}

static bool find_operation_by_name(parser_context* context, const char* func_name, operation_type* found_op) //используется в GetV
{
    assert(context);
    assert(func_name);
    assert(found_op);

    unsigned int func_hash = compute_hash(func_name);

    for (size_t j = 0; j < context -> operations_count; j++)
    {
        if (func_hash == context -> operations[j].hash)
        {
            *found_op = context -> operations[j].op_value;
            return true;
        }
    }

    return false;
}

node_t* get_function(const char** string, parser_context* context)
{
    assert(string);
    assert(context);

    initialize_operation_hashes(context);

    const char* original_pos = *string;

    char func_name[MAX_FUNC_NAME_LENGTH] = {0};

    int chars_read = 0;
    assert(MAX_FUNC_NAME_LENGTH == 256);
    if(sscanf(*string, "%255[a-z]%n", func_name, &chars_read) == 1)
    {
        *string += chars_read;
    }
    else
    {
        *string = original_pos;
        return NULL;
    }

    operation_type found_op = OP_ADD;
    bool found = find_operation_by_name(context, func_name, &found_op);

    if (!found)
    {
        *string = original_pos;
        return NULL;
    }

    node_t* arg = get_P(string, context);
    if (!arg)
    {
        *string = original_pos;
        return NULL;
    }

    return CREATE_UNARY_OP(found_op, arg);
}

void syntax_error()
{
    printf("Syntax error!\n");
}

// ==================== UNDEF MACROS ====================
#undef CREATE_NUM
#undef CREATE_OP
#undef CREATE_UNARY_OP
#undef CREATE_VAR
#undef CHECK_AND_CREATE
#undef RELEASE_IF_NULL
