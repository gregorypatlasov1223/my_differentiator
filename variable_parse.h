#ifndef VARIABLE_PARSE_H_
#define VARIABLE_PARSE_H_

#include <stdlib.h>

#include "tree_error_types.h"

const int MAX_VARIABLE_LENGTH      = 32;
const int MAX_NUMBER_OF_VARIABLES  = 64;

#define DIFFERENT_HASHES -1

struct variable_t
{
    char   name[MAX_VARIABLE_LENGTH];
    size_t hash;
    double value;
    bool   is_defined;
};

struct variable_table
{
    variable_t* variables;
    int         number_of_variables;
    bool        is_sorted;
};

void init_variable_table(variable_table* ptr_table);
int find_variable_by_name(variable_table* ptr_table, const char* name_of_variable);
int find_variable_by_hash(variable_table* ptr_table, size_t hash, const char* name_of_variable);
tree_error_type add_variable(variable_table* ptr_table, const char* name_of_variable);
tree_error_type set_variable_value(variable_table* ptr_table, const char* name_of_variable, double value);
tree_error_type get_variable_value(variable_table* ptr_table, const char* name_of_variable, double* value);
tree_error_type request_variable_value(variable_table* ptr_table, const char* variable_name);
void sort_variable_table(variable_table* ptr_table);
int compare_variables_by_hash(const void* first_var, const void* second_var);
void destroy_variable_table(variable_table* ptr_table);


#endif //VARIABLE_PARSE_H_

