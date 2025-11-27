#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "latex_dump.h"
#include "tree_common.h"

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
                    *position += snprintf(buffer + *position, buffer_size - *position, " + ");
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
    fprintf(file, "\\begin{dmath*}\n");
    fprintf(file, "%s\n", expression);
    fprintf(file, "\\end{dmath*}\n\n");

    fprintf(file, "Result:\n");
    fprintf(file, "\\begin{dmath*}\n");
    fprintf(file, "%.6f\n", result_value);
    fprintf(file, "\\end{dmath*}\n\n");

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
    fprintf(file, "\\begin{dmath*}\n");
    fprintf(file, "%s = %s\n", derivative_notation, derivative_expression);
    fprintf(file, "\\end{dmath*}\n\n");

    fprintf(file, "Value of derivative at point:\n");
    fprintf(file, "\\begin{dmath*}\n");
    fprintf(file, "%s = %.6f\n", derivative_notation, derivative_result);
    fprintf(file, "\\end{dmath*}\n\n");

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

    for (int i = 0; i < var_table -> number_of_variables; i++)
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

    fprintf(file, "\\documentclass[12pt]{article}\n");
    fprintf(file, "\\usepackage[utf8]{inputenc}\n");
    fprintf(file, "\\usepackage{amsmath}\n");
    fprintf(file, "\\usepackage{amssymb}\n");
    fprintf(file, "\\usepackage{breqn}\n");  // пакет для автоматического разбиения
    fprintf(file, "\\usepackage[margin=2.5cm]{geometry}\n");
    fprintf(file, "\\usepackage{parskip}\n");
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

    fprintf(file, "\\documentclass[12pt]{article}\n");
    fprintf(file, "\\usepackage[utf8]{inputenc}\n");
    fprintf(file, "\\usepackage{amsmath}\n");
    fprintf(file, "\\usepackage{amssymb}\n");
    fprintf(file, "\\usepackage{breqn}\n");  // пакет для автоматического разбиения
    fprintf(file, "\\usepackage[margin=2.5cm]{geometry}\n");
    fprintf(file, "\\usepackage{parskip}\n");
    fprintf(file, "\\allowdisplaybreaks\n");  // разрешаем разрывы страниц внутри формул
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
