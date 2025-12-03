#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "dump.h"
#include "io_diff.h"
#include "tree_base.h"
#include "operations.h"

char* read_expression_from_file(const char* filename)
{
    FILE* file = fopen(filename, "r");
    if (!file)
    {
        printf("Error: cannot open file %s\n", filename);
        return NULL;
    }

    size_t file_size = get_file_size(file);

    if (file_size <= 0)
    {
        fclose(file);
        return NULL;
    }

    char* expression = (char*)calloc(file_size + 2, sizeof(char)); // +2 для $ и \0
    if (!expression)
    {
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(expression, 1, file_size, file);
    expression[bytes_read] = '\0';
    fclose(file);

    if (bytes_read > 0)
    {
        if (expression[bytes_read - 1] == '\n')
        {
            expression[bytes_read - 1] = '$';
            expression[bytes_read] = '\0';
        }
        else
        {
            expression[bytes_read] = '$';
            expression[bytes_read + 1] = '\0';
        }
    }
    else
    {
        expression[0] = '$';
        expression[1] = '\0';
    }

    return expression;
}


size_t get_file_size(FILE *file)
{
    assert(file != NULL);

    struct stat stat_buffer = {};

    int file_descriptor = fileno(file);
    if (file_descriptor == -1)
    {
        fprintf(stderr, "Error: Cannot get file descriptor\n");
        return 0;
    }

    if (fstat(file_descriptor, &stat_buffer) != 0)
    {
        fprintf(stderr, "Error: Cannot get file stats\n");
        return 0;
    }

    return (size_t)stat_buffer.st_size;
}


void skip_spaces(const char* buffer, size_t* pos) 
{
    while (isspace(buffer[*pos]))
        (*pos)++;
}
/*

void init_load_progress(load_progress* progress)
{
    assert(progress != NULL);

    progress -> size = 0;
    progress -> capacity = 10;
    progress -> current_depth = 0;
    progress -> items = (node_depth_info*)calloc(progress -> capacity, sizeof(node_depth_info));
}


void add_node_to_load_progress(load_progress* progress, node_t* node, size_t depth)
{
    assert(node     != NULL);
    assert(progress != NULL);

    if (progress -> size >= progress -> capacity)
    {
        size_t new_capacity = progress -> capacity * COEFFICIENT;
        node_depth_info* new_items = (node_depth_info*)realloc(progress -> items, new_capacity * sizeof(node_depth_info));

        if (new_items == NULL)
        {
            fprintf(stderr, "Error: Failed to reallocate memory in add_node_to_load_progress\n");
            return;
        }

        progress -> items = new_items;
        progress -> capacity = new_capacity;
    }

    progress -> items[progress -> size].node = node;
    progress -> items[progress -> size].depth = depth;
    progress -> size++;
}


void free_load_progress(load_progress* progress)
{
    free(progress -> items);

    progress -> items = NULL;
    progress -> size  = 0;
    progress -> capacity = 0;
    progress -> current_depth = 0;

}


node_type determine_node_type(const char* token)
{
    if (strcmp(token, "+") == 0   || strcmp(token, "-") == 0    ||
        strcmp(token, "*") == 0   || strcmp(token, "/") == 0    ||
        strcmp(token, "sin") == 0 || strcmp(token, "cos") == 0)
    {
        return NODE_OP;
    }
    else if (isdigit(token[0]) || (token[0] == '-' && isdigit(token[1])))
    {
        return NODE_NUM;
    }
    else
        return NODE_VAR;
}


void skip_spaces(const char* buffer, size_t* position)
{
    while (isspace(buffer[*position]))
        (*position)++;
}


char* read_token(char* buffer, size_t* position)
{
    assert(buffer   != NULL);
    assert(position != NULL);

    skip_spaces(buffer, position);
    if (buffer[*position] == '\0')
        return NULL;

    size_t start = *position;

    while(buffer[*position] != '\0' && buffer[*position] != ' '  &&
          buffer[*position] != '\t' && buffer[*position] != '\n' &&
          buffer[*position] != '\r' && buffer[*position] != '('  &&
          buffer[*position] != ')')
    {
        (*position)++;
    }

    if ((*position) == start)
        return NULL;

    size_t length = *position - start;

    char* token = (char*)calloc(length + 1, sizeof(char));
    if (token == NULL)
        return NULL;

    strncpy(token, buffer + start, length);
    token[length] = '\0';

    return token;
}


node_t* read_node_from_buffer(tree_t* tree, char* buffer, size_t buffer_length, size_t* position,
                              node_t* parent, int depth, load_progress* progress)
{

    assert(tree     != NULL);
    assert(buffer   != NULL);
    assert(position != NULL);
    assert(progress != NULL);

    skip_spaces(buffer, position);

    if (buffer[*position] == '\0')
        return NULL;

    if (strncmp(buffer + *position, "nul", sizeof("nul") - 1) == 0) // не забываем про \0
    {
        *position += sizeof("nul") - 1;
        return NULL;
    }

    if (buffer[*position] != '(')
        return NULL;

    (*position)++;

    skip_spaces(buffer, position);

    char* string_ptr = read_token(buffer, position);
    if (string_ptr == NULL)
        return NULL;

    node_t* node = create_node_from_token(string_ptr, parent);
    free(string_ptr);

    if (node == NULL)
        return NULL;

    if (tree != NULL)
        tree -> size++;

    if (progress != NULL)
    {
        add_node_to_load_progress(progress, node, depth);
        tree_load_dump(tree, "differentiator_parse", buffer, buffer_length, *position, progress, "Node created");
    }

    skip_spaces(buffer, position);

    node -> left = read_node_from_buffer(tree, buffer, buffer_length, position, node, depth + 1, progress);

    skip_spaces(buffer, position);

    node->right = read_node_from_buffer(tree, buffer, buffer_length, position, node, depth + 1, progress);

    skip_spaces(buffer, position);

    if (buffer[*position] != ')')
    {
        tree_destroy_recursive(node);
        return NULL;
    }

    (*position)++;

    if (progress != NULL)
        tree_load_dump(tree, "differentiator_parse", buffer, buffer_length, *position, progress, "Subtree complete");

    return node;
}


tree_error_type tree_load(tree_t* tree, const char* filename)
{
    if (tree == NULL || filename == NULL)
        return TREE_ERROR_NULL_PTR;

    FILE* file = fopen(filename, "r");
    if (file == NULL)
        return TREE_ERROR_OPENING_FILE;

    size_t file_size = get_file_size(file);
    if (file_size == 0)
    {
        fclose(file);
        return TREE_ERROR_FORMAT;
    }

    char* file_buffer = (char*)calloc(file_size + 1, sizeof(char));
     if (file_buffer == NULL)
    {
        fclose(file);
        return TREE_ERROR_ALLOCATION;
    }

    size_t bytes_read = fread(file_buffer, sizeof(char), file_size, file);
    file_buffer[bytes_read] = '\0';
    fclose(file);

    load_progress progress = {};
    init_load_progress(&progress);

    size_t position = 0;
    size_t buffer_length = strlen(file_buffer);

    tree -> root = read_node_from_buffer(tree, file_buffer, buffer_length, &position,
                              NULL, 0, &progress);
    if (tree -> root == NULL)
    {
        free(file_buffer);
        free_load_progress(&progress);
        return TREE_ERROR_FORMAT;
    }

    tree -> file_buffer = file_buffer;
    tree_dump(tree, "tree_ready");

    free_load_progress(&progress);
    return TREE_ERROR_NO;
}

*/



