#ifndef NEW_INPUT_H_
#define NEW_INPUT_H_

#include "operations.h"
#include "tree_common.h"
#include "variable_parse.h"

struct parser_context
{
    variable_table* var_table;
    operation_info* operations;
    size_t operations_count;
    bool hashes_initialized;
};

node_t* get_G(const char** s, variable_table* var_table);
node_t* get_E(const char** s, parser_context* context);
node_t* get_T(const char** s, parser_context* context);
node_t* get_F(const char** s, parser_context* context);
node_t* get_P(const char** s, parser_context* context);
node_t* get_N(const char** s);
node_t* get_V(const char** s, parser_context* context);
node_t* get_function(const char** s, parser_context* context);
void syntax_error();

#endif // NEW_INPUT_H_
