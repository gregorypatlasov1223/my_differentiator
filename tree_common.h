#ifndef TREE_COMMON_H_
#define TREE_COMMON_H_

#include <stdlib.h>

#define MAX_LENGTH_OF_ADDRESS 128
#define MAX_LENGTH_OF_ANSWER 1024
#define MAX_PATH_DEPTH 512
#define MAX_LENGTH_OF_FILENAME 256
#define MAX_LENGTH_OF_SYSTEM_COMMAND 512

#define OPERATION_FAILED -1
#define COMPLETED_SUCCESSFULLY 1

#define BASE_EDGE_LENGTH 1.0
#define DEPTH_SPREAD_FACTOR 0.5
#define ZERO_RANK 0

const char* const DEFAULT_DATA_BASE_FILENAME = "differenciator_tree.txt";
const int MAX_LENGTH_OF_TEX_EXPRESSION       = 8192;
const int MAX_NUMBER_OF_DERIVATIVE           = 4;


enum node_type
{
    NODE_OP,
    NODE_VAR,
    NODE_NUM
};

enum operation_type
{
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_SIN,
    OP_COS,
};

struct variable_definition
{
    char* name;
    size_t hash;
};

union value_of_tree_element
{
    double              num_value;
    operation_type      op_value;
    variable_definition var_definition;
};

struct node_t
{
    value_of_tree_element data;
    node_type             type;
    node_t*               left;
    node_t*               right;
    node_t*               parent;
};

struct tree_t
{
    node_t* root;
    size_t size;
    char* file_buffer;
};

struct node_depth_info
{
    node_t* node;
    size_t depth;
};

struct load_progress
{
    node_depth_info* items;
    size_t size;
    size_t capacity;
    size_t current_depth;
};

#endif // TREE_COMMON_H_
