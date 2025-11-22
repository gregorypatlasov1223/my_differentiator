#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "tree_base.h"
#include "variable_parse.h"

void init_variable_table(variable_table* ptr_table)
{
    if (ptr_table == NULL)
    {
        fprintf(stderr, "Error: NULL pointer passed to init_variable_table\n");
        return;
    }

    ptr_table -> number_of_variables = 0;
    ptr_table -> is_sorted = true;
    ptr_table -> variables = (variable_t*)calloc(MAX_NUMBER_OF_VARIABLES, sizeof(variable_t));

    for (size_t index_of_variable = 0; index_of_variable < MAX_NUMBER_OF_VARIABLES; index_of_variable++)
    {
        ptr_table -> variables[index_of_variable].name[0]    = '\0';
        ptr_table -> variables[index_of_variable].value      = 0.0;
        ptr_table -> variables[index_of_variable].hash       = 0;
        ptr_table -> variables[index_of_variable].is_defined = false;
    }
}


int find_variable_by_name(variable_table* ptr_table, const char* name_of_variable)
{
    assert(ptr_table        != NULL);
    assert(name_of_variable != NULL);

    printf("DEBUG find_variable_by_name: looking for '%s'\n", name_of_variable);
    printf("DEBUG find_variable_by_name: number_of_variables = %d\n", ptr_table->number_of_variables);

    for (int i = 0; i < ptr_table->number_of_variables; i++)
    {
        printf("DEBUG find_variable_by_name: table[%d] = '%s' (strcmp result: %d)\n",
               i, ptr_table->variables[i].name,
               strcmp(ptr_table->variables[i].name, name_of_variable));

        if (strcmp(ptr_table->variables[i].name, name_of_variable) == 0)
        {
            printf("DEBUG find_variable_by_name: found at index %d\n", i);
            return i;
        }
    }

    printf("DEBUG find_variable_by_name: variable not found\n");
    return OPERATION_FAILED;
}


int find_variable_by_hash(variable_table* ptr_table, size_t hash, const char* name_of_variable)
{
    assert(ptr_table        != NULL);
    assert(name_of_variable != NULL);

    if(!ptr_table -> is_sorted)
        sort_variable_table(ptr_table);

    if (ptr_table -> number_of_variables == 0)
        return OPERATION_FAILED;

    size_t left = 0;
    size_t right = ptr_table -> number_of_variables - 1;

    while (left <= right)
    {
        size_t middle = left + (right - left) / 2;
        if (ptr_table -> variables[middle].hash == hash)
        {
            size_t same_hash_start = middle;
            size_t same_hash_end   = middle;

            while (same_hash_start > 0 && ptr_table -> variables[same_hash_start - 1].hash == hash)
                same_hash_start--;

            while (same_hash_end < ptr_table -> number_of_variables - 1 &&
                  ptr_table -> variables[same_hash_end + 1].hash == hash)
                same_hash_end++;

            for (int i = same_hash_start; i < same_hash_end; i++)
            {
                if (strcmp(ptr_table -> variables[i].name, name_of_variable) == 0)
                    return (int)i;
            }
            return OPERATION_FAILED;
        }
        else if (ptr_table -> variables[middle].hash < hash)
            left = middle + 1;
        else
        {
            if (middle == 0) break;
            right = middle - 1;
        }
    }
    return OPERATION_FAILED;
}


tree_error_type add_variable(variable_table* ptr_table, const char* name_of_variable)
{
    if (ptr_table == NULL || name_of_variable == NULL)
        return TREE_ERROR_NULL_PTR;

    printf("DEBUG add_variable: adding variable '%s'\n", name_of_variable);
    printf("DEBUG add_variable: current number_of_variables = %d\n", ptr_table->number_of_variables);

    if (ptr_table->number_of_variables >= MAX_NUMBER_OF_VARIABLES)
    {
        printf("DEBUG add_variable: table full\n");
        return TREE_ERROR_VARIABLE_TABLE;
    }

    if (find_variable_by_name(ptr_table, name_of_variable) != OPERATION_FAILED)
    {
        printf("DEBUG add_variable: variable already exists\n");
        return TREE_ERROR_REDEFINITION_VARIABLE;
    }

    int index = ptr_table->number_of_variables;

    // Копируем имя переменной
    strncpy(ptr_table->variables[index].name, name_of_variable, MAX_VARIABLE_LENGTH - 1);
    ptr_table->variables[index].name[MAX_VARIABLE_LENGTH - 1] = '\0';

    ptr_table->variables[index].value = 0.0;
    ptr_table->variables[index].hash = compute_hash(name_of_variable);
    ptr_table->variables[index].is_defined = false; // Устанавливаем в false, значение еще не задано

    ptr_table->number_of_variables++;
    ptr_table->is_sorted = false;

    printf("DEBUG add_variable: added variable '%s' at index %d\n",
           ptr_table->variables[index].name, index);
    printf("DEBUG add_variable: new number_of_variables = %d\n", ptr_table->number_of_variables);

    return TREE_ERROR_NO;
}


tree_error_type set_variable_value(variable_table* ptr_table, const char* name_of_variable, double value)
{
    if (ptr_table == NULL || name_of_variable == NULL)
        return TREE_ERROR_NULL_PTR;

    printf("DEBUG set_variable_value: start for variable '%s'\n", name_of_variable);
    printf("DEBUG set_variable_value: ptr_table = %p\n", ptr_table);
    printf("DEBUG set_variable_value: number_of_variables = %d\n", ptr_table->number_of_variables);

    // Выводим всю таблицу для отладки
    printf("DEBUG set_variable_value: current table state:\n");
    for (int i = 0; i < ptr_table->number_of_variables; i++)
    {
        printf("  [%d] name='%s', value=%.6f, is_defined=%d\n",
               i, ptr_table->variables[i].name, ptr_table->variables[i].value, ptr_table->variables[i].is_defined);
    }

    int index = find_variable_by_name(ptr_table, name_of_variable);
    if (index == OPERATION_FAILED)
    {
        printf("DEBUG set_variable_value: variable '%s' not found in table\n", name_of_variable);
        return TREE_ERROR_VARIABLE_NOT_FOUND;
    }

    printf("DEBUG set_variable_value: variable found at index %d\n", index);
    ptr_table->variables[index].value = value;
    ptr_table->variables[index].is_defined = true;

    printf("DEBUG set_variable_value: set value to %.6f\n", value);
    return TREE_ERROR_NO;
}


tree_error_type get_variable_value(variable_table* ptr_table, const char* name_of_variable, double* value)
{
    if (ptr_table == NULL || name_of_variable == NULL || value == NULL)
        return TREE_ERROR_NULL_PTR;

    printf("DEBUG get_variable_value: looking for variable '%s'\n", name_of_variable);

    int index = find_variable_by_name(ptr_table, name_of_variable);

    if (index == OPERATION_FAILED)
    {
        printf("DEBUG get_variable_value: variable not found\n");
        return TREE_ERROR_VARIABLE_NOT_FOUND;
    }

    if (!ptr_table -> variables[index].is_defined)
    {
        printf("DEBUG get_variable_value: variable found but undefined\n");
        return TREE_ERROR_VARIABLE_UNDEFINED;
    }

    *value = ptr_table -> variables[index].value;
    printf("DEBUG get_variable_value: variable found, value = %f\n", *value);
    return TREE_ERROR_NO;
}


tree_error_type request_variable_value(variable_table* ptr_table, const char* variable_name)
{
    if (ptr_table == NULL || variable_name == NULL)
        return TREE_ERROR_NULL_PTR;

    printf("DEBUG request_variable_value: start for variable '%s'\n", variable_name);

    printf("Enter a value for the variable '%s': ", variable_name);
    fflush(stdout);  // Важно: сбрасываем буфер вывода

    double value = 0.0;

    if (scanf("%lf", &value) != 1)
    {
        clear_input_buffer();
        printf("RequestVariableValue: Invalid input\n");
        return TREE_ERROR_INVALID_INPUT;
    }

    clear_input_buffer();

    printf("DEBUG request_variable_value: read value %f\n", value);

    tree_error_type result = set_variable_value(ptr_table, variable_name, value);
    printf("DEBUG request_variable_value: set_variable_value returned %d\n", result);

    return result;
}


void sort_variable_table(variable_table* ptr_table)
{
    assert(ptr_table != NULL);

    if (ptr_table -> number_of_variables > 1)
    {
        qsort(ptr_table -> variables, ptr_table -> number_of_variables, sizeof(variable_t), compare_variables_by_hash);
        ptr_table -> is_sorted = true;
    }
}


int compare_variables_by_hash(const void* first_var, const void* second_var)
{
    const variable_t* varA = (const variable_t*)first_var;
    const variable_t* varB = (const variable_t*)second_var;

    if (varA -> hash != varB -> hash)
        return DIFFERENT_HASHES;

    return strcmp(varA -> name, varB -> name);
}


void destroy_variable_table(variable_table* ptr_table)
{
    assert(ptr_table != NULL);

    if (ptr_table -> variables != NULL)
    {
        free(ptr_table -> variables);
        ptr_table -> variables = NULL;
    }

    ptr_table -> number_of_variables = 0;
    ptr_table -> is_sorted = true;
}

