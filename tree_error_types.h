#ifndef TREE_ERROR_TYPES_H_
#define TREE_ERROR_TYPES_H_


enum tree_verify_result
{
    TREE_VERIFY_SUCCESS        = 0,
    TREE_VERIFY_NULL_PTR       = 1,
    TREE_VERIFY_INVALID_PARENT = 2,
    TREE_VERIFY_CYCLE_DETECTED = 3
};


enum tree_error_type
{
    TREE_ERROR_NO                    = 0,
    TREE_ERROR_NULL_PTR              = 1,
    TREE_ERROR_ALLOCATION            = 2,
    TREE_ERROR_OPENING_FILE          = 3,
    TREE_ERROR_INVALID_NODE          = 4,
    TREE_ERROR_STRUCTURE             = 5,
    TREE_ERROR_FORMAT                = 6,
    TREE_ERROR_IO                    = 7,
    TREE_ERROR_ALREADY_INITIALIZED   = 8,

    TREE_ERROR_VARIABLE_TABLE        = 9,
    TREE_ERROR_REDEFINITION_VARIABLE = 10,
    TREE_ERROR_VARIABLE_NOT_FOUND    = 11,
    TREE_ERROR_VARIABLE_UNDEFINED    = 12,
    TREE_ERROR_INVALID_INPUT         = 13,
    TREE_ERROR_DIVISION_BY_ZERO      = 14,
    TREE_ERROR_UNKNOWN_OPERATION     = 15,
    TREE_ERROR_NO_VARIABLES          = 16
};

#endif // TREE_ERROR_TYPES_H_
