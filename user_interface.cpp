#include <stdio.h>
#include <assert.h>

#include "user_interface.h"
#include "tree_error_types.h"


const char* get_data_base_filename(int argc, const char** argv)
{
    assert(argv != NULL);

    return (argc < 1) ? DEFAULT_DATA_BASE_FILENAME : argv[1];
}


const char* tree_error_translator(tree_error_type error)
{
    switch (error)
    {
        case TREE_ERROR_NO:                 return "Successful completion";
        case TREE_ERROR_DIVISION_BY_ZERO:   return "Division by zero!";
        case TREE_ERROR_VARIABLE_NOT_FOUND: return "The variable was not found!";
        case TREE_ERROR_VARIABLE_UNDEFINED: return "The variable is not defined!";
        case TREE_ERROR_INVALID_INPUT:      return "Incorrect input of the variable value!";
        case TREE_ERROR_NULL_PTR:           return "The null pointer!";
        case TREE_ERROR_UNKNOWN_OPERATION:  return "Unknown operation!";
        case TREE_ERROR_IO:                 return "IO error (file not found or unavailable)";
        case TREE_ERROR_FORMAT:             return "File format error";
        case TREE_ERROR_ALLOCATION:         return "Memory allocation error";
        case TREE_ERROR_NO_VARIABLES:       return "No variables found";
        default:                            return "Unknown error";
    }
}


void print_tree_error(tree_error_type error)
{
    printf("%s\n", tree_error_translator(error));
}
