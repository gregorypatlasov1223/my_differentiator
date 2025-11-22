#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "tree_base.h"


bool is_leaf(node_t* node)
{
    if (node == NULL)
        return false;

    return (node -> left == NULL && node -> right == NULL);
}


tree_error_type tree_destroy_recursive(node_t* node)
{
    if (node == NULL)
        return TREE_ERROR_NO;

    tree_destroy_recursive(node -> left);
    tree_destroy_recursive(node -> right);

    if (node -> type == NODE_VAR && node -> data.var_definition.name != NULL)
        free(node -> data.var_definition.name);

    free(node);

    return TREE_ERROR_NO;
}


tree_error_type tree_constructor(tree_t* tree)
{
    if (tree == NULL)
        return TREE_ERROR_NULL_PTR;

    tree -> size        = 0;
    tree -> root        = NULL;
    tree -> file_buffer = NULL;

    return TREE_ERROR_NO;
}


tree_error_type tree_destructor(tree_t* tree)
{
    if (tree == NULL)
        return TREE_ERROR_NULL_PTR;

    tree_destroy_recursive(tree -> root);
    if (tree -> file_buffer != NULL)
    {
        free(tree -> file_buffer);
        tree -> file_buffer = NULL;
    }

    tree -> size = 0;
    tree -> root = NULL;

    return TREE_ERROR_NO;
}


node_t* create_node_from_token(const char* token, node_type type, node_t* parent)
{
    node_t* node = (node_t*)calloc(1, sizeof(node_t));
    if (node == NULL)
        return NULL;

    node -> type   = type;
    node -> parent = parent;

    switch (type)
    {
        case NODE_OP:
            if (strcmp(token, "+") == 0)
                node -> data.op_value = OP_ADD;
            else if (strcmp(token, "-") == 0)
                node -> data.op_value = OP_SUB;
            else if (strcmp(token, "*") == 0)
                node -> data.op_value = OP_MUL;
            else if (strcmp(token, "/") == 0)
                node -> data.op_value = OP_DIV;
            else if (strcmp(token, "sin") == 0)
                node -> data.op_value = OP_SIN;
            else if (strcmp(token, "cos") == 0)
                node -> data.op_value = OP_COS;
            else
            {
                free(node);
                return NULL;
            }
            break;

        case NODE_NUM:
            node -> data.num_value = atof(token);
            break;

        case NODE_VAR:
            node -> data.var_definition.hash = compute_hash(token);
            node -> data.var_definition.name = strdup(token);

            if (node -> data.var_definition.name == NULL)
            {
                free(node);
                return NULL;
            }
            break;

        default:
            free(node);
            return NULL;
    }

    node -> left = NULL;
    node -> right = NULL;

    return node;
}


size_t compute_hash(const char* string)
{
    assert(string);

    size_t hash = 5381;
    size_t index = 0;

    while (string[index] != '\0')
    {
        hash = hash * 33 + (size_t)string[index];
        index++;
    }

    return hash;
}


void clear_input_buffer()
{
    int symbol = 0;
    while ((symbol = getchar()) != '\n' && symbol != EOF);
}













