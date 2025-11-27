#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "latex_dump.h"
#include "tree_common.h"


int is_binary_operator(const char* expression, int i, int length)
{
    if (i <= 0) return 0;
    char prev = expression[i-1];

    return (prev != '(' && prev != ')' &&
            prev != '+' && prev != '-' &&
            prev != '*' && prev != '/');
}


void add_part(char** parts, int* part_count, const char* expression, int start, int end)
{
    int part_length = end - start;
    if (part_length > 0 && *part_count < MAX_PARTS)
    {
        parts[*part_count] = (char*)calloc(part_length + 1, sizeof(char));

        strncpy(parts[*part_count], expression + start, part_length);
        parts[*part_count][part_length] = '\0';

        (*part_count)++;
    }
}


int split_by_operators(const char* expression, char** parts, int* part_count,
                      const char* operators)
{
    int depth = 0;
    int start = 0;
    int was_split = 0;
    int length = strlen(expression);

    for (int i = 0; i < length && *part_count < MAX_PARTS - 1; i++)
    {
        if (expression[i] == '(')
            depth++;
        else if (expression[i] == ')')
            depth--;
        else if (depth == 0 && strchr(operators, expression[i]) != NULL)
        {
            if (is_binary_operator(expression, i, length))
            {
                if (expression[i] == '*')
                {
                    add_part(parts, part_count, expression, start, i);
                    start = i; // Оператор будет в начале следующей части
                    was_split = 1;
                }
                else
                {
                    add_part(parts, part_count, expression, start, i + 1);
                    start = i + 1;
                    was_split = 1;
                }
            }
        }
    }

    // Добавляем последнюю часть
    if (was_split || *part_count > 0)
    {
        add_part(parts, part_count, expression, start, length);
        return 1;
    }

    return 0;
}


char** split_expression(const char* expression, int* part_count)
{
    static char* parts[MAX_PARTS];
    *part_count = 0;

    if (split_by_operators(expression, parts, part_count, "+-"))
        return parts;

    if (split_by_operators(expression, parts, part_count, "*"))
        return parts;

    return NULL;
}


void free_parts(char** parts, int part_count)
{
    for (int i = 0; i < part_count; i++)
    {
        free(parts[i]);
    }
}


//=========================== END SPLIT FUNCTIONS ==================================================


void tree_to_string_simple(node_t* node, char* buffer, size_t* position, int buffer_size)
{
    assert(node != NULL);

    switch(node -> type)
    {
        case NODE_NUM:
            *position += snprintf(buffer + *position, buffer_size - *position, "%.2f", node -> data.num_value);
            break;

        case NODE_VAR:
            if (node -> data.var_definition.name != NULL)
                *position += snprintf(buffer + *position, buffer_size - *position, "%s", node -> data.var_definition.name);
            else
                *position += snprintf(buffer + *position, buffer_size - *position, "?");
            break;

        case NODE_OP:
            switch(node -> data.op_value)
            {
                case OP_ADD:
                    tree_to_string_simple(node -> left, buffer, position, buffer_size);
                    *position += snprintf(buffer + *position, buffer_size - *position, " +" );
                    tree_to_string_simple(node -> right, buffer, position, buffer_size);
                    break;
                case OP_SUB:
                    tree_to_string_simple(node -> left, buffer, position, buffer_size);
                    *position += snprintf(buffer + *position, buffer_size - *position, " - ");
                    tree_to_string_simple(node -> right, buffer, position, buffer_size);
                    break;
                case OP_MUL:
                    tree_to_string_simple(node -> left, buffer, position, buffer_size);
                    *position += snprintf(buffer + *position, buffer_size - *position, " \\cdot ");
                    tree_to_string_simple(node -> right, buffer, position, buffer_size);
                    break;
                case OP_DIV:
                    *position += snprintf(buffer + *position, buffer_size - *position, "\\frac{");
                    tree_to_string_simple(node -> left, buffer, position, buffer_size);
                    *position += snprintf(buffer + *position, buffer_size - *position, "}{");
                    tree_to_string_simple(node -> right, buffer, position, buffer_size);
                    *position += snprintf(buffer + *position, buffer_size - *position, "}");
                    break;
                case OP_SIN:
                    *position += snprintf(buffer + *position, buffer_size - *position, "\\sin(");
                    tree_to_string_simple(node -> right, buffer, position, buffer_size);
                    *position += snprintf(buffer + *position, buffer_size - *position, ")");
                    break;
                case OP_COS:
                    *position += snprintf(buffer + *position, buffer_size - *position, "\\cos(");
                    tree_to_string_simple(node -> right, buffer, position, buffer_size);
                    *position += snprintf(buffer + *position, buffer_size - *position, ")");
                    break;
                default:
                    *position += snprintf(buffer + *position, buffer_size - *position, "?");
            }
            break;

        default:
            *position += snprintf(buffer + *position, buffer_size - *position, "?");
    }
}


tree_error_type dump_original_function(FILE* file, tree_t* tree, double result_value)
{
    if (file == NULL || tree == NULL)
        return TREE_ERROR_NULL_PTR;

    char expression[MAX_LENGTH_OF_TEX_EXPRESSION] = {};
    size_t position = 0;

    tree_to_string_simple(tree -> root, expression, &position, sizeof(expression));

    fprintf(file, "\\section*{Mathematical Expression}\n\n");
    fprintf(file, "Expression:\n");

//     int part_count = 0;
//     char** parts = split_expression(expression, &part_count);
//
//     if (parts != NULL && part_count > 1)
//     {
//         fprintf(file, "\\begin{multline*}\n");
//         fprintf(file, "%s", parts[0]);
//
//         for (int i = 1; i < part_count; i++)
//         {
//             fprintf(file, " \\\\\n%s", parts[i]);
//         }
//
//         fprintf(file, "\n\\end{multline*}\n\n");
//         free_parts(parts, part_count);
//     }
//     else
//     {
//         fprintf(file, "\\[ %s \\]\n\n", expression);
//     }

    fprintf(file, "Result:\n");
    fprintf(file, "\\[ %.6f \\]\n\n", result_value);

    return TREE_ERROR_NO;
}


tree_error_type dump_derivative(FILE* file, tree_t* derivative_tree, double derivative_result, int derivative_order)
{
    if (file == NULL || derivative_tree == NULL)
        return TREE_ERROR_NULL_PTR;

    char derivative_expression[MAX_LENGTH_OF_TEX_EXPRESSION] = {};
    size_t position = 0;

    tree_to_string_simple(derivative_tree -> root, derivative_expression, &position, sizeof(derivative_expression));

    const char* derivative_notation = NULL;
    char custom_notation[MAX_LENGTH_OF_TEX_EXPRESSION] = {};

    if (derivative_order == 1)
    {
        derivative_notation = "f'(x)";
    }
    else if (derivative_order == 2)
    {
        derivative_notation = "f''(x)";
    }
    else if (derivative_order == 3)
    {
        derivative_notation = "f'''(x)";
    }
    else
    {
        snprintf(custom_notation, sizeof(custom_notation), "f^{(%d)}(x)", derivative_order);
        derivative_notation = custom_notation;
    }

    fprintf(file, "\\subsection*{Derivative of Order %d}\n", derivative_order);
    fprintf(file, "Derivative expression:\n");

    int part_count = 0;
    char** parts = split_expression(derivative_expression, &part_count);

    if (parts != NULL && part_count > 1)
    {
        fprintf(file, "\\begin{multline*}\n");
        fprintf(file, "%s = %s", derivative_notation, parts[0]);

        for (int i = 1; i < part_count; i++)
        {
            fprintf(file, " \\\\\n%s", parts[i]);
        }

        fprintf(file, "\n\\end{multline*}\n\n");
        free_parts(parts, part_count);
    }
    else
        fprintf(file, "\\[ %s = %s \\]\n\n", derivative_notation, derivative_expression);

    fprintf(file, "Value of derivative at point: \\[ %s = %.6f \\]\n\n", derivative_notation, derivative_result);

    return TREE_ERROR_NO;
}


tree_error_type dump_variable_table(FILE* file, variable_table* var_table)
{
    if (file == NULL || var_table == NULL)
        return TREE_ERROR_NULL_PTR;

    if (var_table -> number_of_variables <= 0)
        return TREE_ERROR_NO;

    fprintf(file, "\\section*{Variables}\n");
    fprintf(file, "\\begin{tabular}{|c|c|}\n");
    fprintf(file, "\\hline\n");
    fprintf(file, "Name & Value \\\\\n");
    fprintf(file, "\\hline\n");

    for (int i = 0; i < var_table -> number_of_variables; i++) //вывод таблицы переменных
    {
        fprintf(file, "%s & %.4f \\\\\n", var_table -> variables[i].name, var_table -> variables[i].value);
    }

    fprintf(file, "\\hline\n");
    fprintf(file, "\\end{tabular}\n\n");

    return TREE_ERROR_NO;
}


tree_error_type generate_latex_dump(tree_t* tree, variable_table* var_table, const char* filename, double result_value)
{
    if (tree == NULL || var_table == NULL || filename == NULL)
        return TREE_ERROR_NULL_PTR;

    FILE* file = fopen(filename, "w");
    if (file == NULL)
        return TREE_ERROR_IO;

    fprintf(file, "\\documentclass{article}\n");
    fprintf(file, "\\usepackage[utf8]{inputenc}\n");
    fprintf(file, "\\usepackage{amsmath}\n");
    fprintf(file, "\\begin{document}\n\n");

    tree_error_type error = dump_original_function(file, tree, result_value);
    if (error != TREE_ERROR_NO)
    {
        fclose(file);
        return error;
    }

    error = dump_variable_table(file, var_table);
    if (error != TREE_ERROR_NO && error != TREE_ERROR_NO_VARIABLES)
    {
        fclose(file);
        return error;
    }

    fprintf(file, "\\end{document}\n");
    fclose(file);

    printf("Simple LaTeX file created: %s\n", filename);
    printf("To compile: pdflatex %s\n", filename);

    return TREE_ERROR_NO;
}


tree_error_type generate_latex_dump_with_derivatives(tree_t* tree, tree_t** derivative_trees, double* derivative_results,
                                              int derivative_count, variable_table* var_table,
                                              const char* filename, double result_value)
{
    assert(tree               != NULL);
    assert(filename           != NULL);
    assert(var_table          != NULL);
    assert(derivative_trees   != NULL);
    assert(derivative_results != NULL);

    FILE* file = fopen(filename, "w");
    if (file == NULL)
        return TREE_ERROR_IO;

    fprintf(file, "\\documentclass{article}\n");
    fprintf(file, "\\usepackage[utf8]{inputenc}\n");
    fprintf(file, "\\usepackage{amsmath}\n");
    fprintf(file, "\\begin{document}\n\n");

    fprintf(file, "\\section*{Mathematical Expression Analysis}\n\n");

    tree_error_type error = dump_original_function(file, tree, result_value);
    if (error != TREE_ERROR_NO)
    {
        fclose(file);
        return error;
    }

    for (int i = 0; i < derivative_count; i++)
    {
        error = dump_derivative(file, derivative_trees[i], derivative_results[i], i + 1);
        if (error != TREE_ERROR_NO)
        {
            fclose(file);
            return error;
        }
    }

    error = dump_variable_table(file, var_table);
    if (error != TREE_ERROR_NO && error != TREE_ERROR_NO_VARIABLES)
    {
        fclose(file);
        return error;
    }

    fprintf(file, "\\end{document}\n");
    fclose(file);

    printf("LaTeX file with %d derivatives created: %s\n", derivative_count, filename);
    printf("To compile: pdflatex %s\n", filename);

    return TREE_ERROR_NO;
}
