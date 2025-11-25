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













