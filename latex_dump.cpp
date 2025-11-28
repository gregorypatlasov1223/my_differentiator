#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "latex_dump.h"


static void escape_latex_special_chars(const char* input, char* output, size_t output_size) {
    if (input == NULL || output == NULL || output_size == 0) return;

    size_t j = 0;
    for (size_t i = 0; input[i] != '\0' && j < output_size - 1; i++) {
        switch (input[i]) {
            case '^':
                // Обработка степеней в тексте - оборачиваем в математический режим
                if (isdigit(input[i+1])) {
                    // Найдена степень с цифрой - оборачиваем в $...$
                    if (j + 3 < output_size - 1) {
                        output[j++] = '$';
                        output[j++] = '^';
                        output[j++] = input[++i]; // Пропускаем ^ и берем цифру
                        output[j++] = '$';
                    }
                } else {
                    // Просто символ ^ без цифры - экранируем
                    if (j + 2 < output_size - 1) {
                        output[j++] = '\\';
                        output[j++] = '^';
                    }
                }
                break;
            case '_':
                // Нижнее подчеркивание нужно экранировать
                if (j + 2 < output_size - 1) {
                    output[j++] = '\\';
                    output[j++] = '_';
                }
                break;
            case '%':
                // Процент нужно экранировать
                if (j + 2 < output_size - 1) {
                    output[j++] = '\\';
                    output[j++] = '%';
                }
                break;
            case '&':
                // Амперсанд нужно экранировать
                if (j + 2 < output_size - 1) {
                    output[j++] = '\\';
                    output[j++] = '&';
                }
                break;
            case '#':
                // Решетку нужно экранировать
                if (j + 2 < output_size - 1) {
                    output[j++] = '\\';
                    output[j++] = '#';
                }
                break;
            case '$':
                // Доллар нужно экранировать
                if (j + 2 < output_size - 1) {
                    output[j++] = '\\';
                    output[j++] = '$';
                }
                break;
            case '\\':
                // Обратный слеш нужно экранировать
                if (j + 2 < output_size - 1) {
                    output[j++] = '\\';
                    output[j++] = '\\';
                }
                break;
            case '~':
                // Тильду нужно экранировать
                if (j + 2 < output_size - 1) {
                    output[j++] = '\\';
                    output[j++] = '~';
                }
                break;
            case '{':
                // Фигурные скобки нужно экранировать
                if (j + 2 < output_size - 1) {
                    output[j++] = '\\';
                    output[j++] = '{';
                }
                break;
            case '}':
                // Фигурные скобки нужно экранировать
                if (j + 2 < output_size - 1) {
                    output[j++] = '\\';
                    output[j++] = '}';
                }
                break;
            default:
                // Обычный символ - просто копируем
                output[j++] = input[i];
                break;
        }
    }
    output[j] = '\0';
}

/**
 * @brief Создает безопасное для LaTeX описание
 * @param description Исходное описание
 * @return Указатель на статический буфер с безопасной строкой
 */
static const char* make_latex_safe_description(const char* description) {
    static char safe_buffer[2 * MAX_TEX_DESCRIPTION_LENGTH];
    escape_latex_special_chars(description, safe_buffer, sizeof(safe_buffer));
    return safe_buffer;
}


void tree_to_string_simple(node_t* node, char* buffer, int* position, int buffer_size)
{
    if (node == NULL || *position >= buffer_size - 1)
        return;

    switch (node -> type)
    {
        case NODE_NUM:
            *position += snprintf(buffer + *position, buffer_size - *position, "%.2f", node -> data.num_value);
            break;

        case NODE_VAR:
            if (node -> data.var_definition.name)
            {
                *position += snprintf(buffer + *position, buffer_size - *position, "%s", node -> data.var_definition.name);
            }
            else
            {
                *position += snprintf(buffer + *position, buffer_size - *position, "?");
            }
            break;

        case NODE_OP:
            switch (node -> data.op_value)
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
                case OP_POW:
                    *position += snprintf(buffer + *position, buffer_size - *position, "{");
                    tree_to_string_simple(node -> left, buffer, position, buffer_size);
                    *position += snprintf(buffer + *position, buffer_size - *position, "}^{");
                    tree_to_string_simple(node -> right, buffer, position, buffer_size);
                    *position += snprintf(buffer + *position, buffer_size - *position, "}");
                    break;
                case OP_LN:
                    *position += snprintf(buffer + *position, buffer_size - *position, "\\ln(");
                    tree_to_string_simple(node -> right, buffer, position, buffer_size);
                    *position += snprintf(buffer + *position, buffer_size - *position, ")");
                    break;
                case OP_EXP:
                    *position += snprintf(buffer + *position, buffer_size - *position, "e^{");
                    tree_to_string_simple(node -> right, buffer, position, buffer_size);
                    *position += snprintf(buffer + *position, buffer_size - *position, "}");
                    break;
                default:
                    *position += snprintf(buffer + *position, buffer_size - *position, "?");
            }
            break;

        default:
            *position += snprintf(buffer + *position, buffer_size - *position, "?");
    }
}


tree_error_type start_latex_dump(FILE* file)
{
    if (file == NULL)
        return TREE_ERROR_NULL_PTR;

    fprintf(file, "\\documentclass[12pt]{article}\n");
    fprintf(file, "\\usepackage[utf8]{inputenc}\n");
    fprintf(file, "\\usepackage{amsmath}\n");
    fprintf(file, "\\usepackage{geometry}\n");
    fprintf(file, "\\geometry{a4paper, left=20mm, right=20mm, top=20mm, bottom=20mm}\n");
    fprintf(file, "\\setlength{\\parindent}{0pt}\n");
    fprintf(file, "\\setlength{\\parskip}{1em}\n");
    fprintf(file, "\\begin{document}\n");

    fprintf(file, "\\section*{Mathematical Expression Analysis}\n\n");

    return TREE_ERROR_NO;
}


tree_error_type end_latex_dump(FILE* file)
{
    if (file == NULL)
        return TREE_ERROR_NULL_PTR;

    fprintf(file, "\\end{document}\n");
    return TREE_ERROR_NO;
}


tree_error_type dump_original_function_to_file(FILE* file, tree_t* tree, double result_value)
{
    if (file == NULL || tree == NULL)
        return TREE_ERROR_NULL_PTR;

    char expression[MAX_LENGTH_OF_TEX_EXPRESSION] = {};
    int position = 0;
    tree_to_string_simple(tree -> root, expression, &position, sizeof(expression));

    fprintf(file, "\\subsection*{Original Expression}\n");
    fprintf(file, "Expression: \\[ %s \\]\n\n", expression);
    fprintf(file, "Evaluation result: \\[ %.6f \\]\n\n", result_value);

    return TREE_ERROR_NO;
}


tree_error_type dump_optimization_step_to_file(FILE* file, const char* description, tree_t* tree, double result_value)
{
    if (file == NULL || description == NULL || tree == NULL)
        return TREE_ERROR_NULL_PTR;

    const char* safe_description = make_latex_safe_description(description);

    fprintf(file, "\\subsubsection*{Optimization Step}\n");
    fprintf(file, "It is easy to see that %s:\n\n", description);

    char expression[MAX_LENGTH_OF_TEX_EXPRESSION] = {0};
    int pos = 0;
    tree_to_string_simple(tree->root, expression, &pos, sizeof(expression));

    fprintf(file, "\\[ %s \\]\n\n", expression);
    fprintf(file, "Result after simplification: \\[ %.6f \\]\n\n", result_value);
    fprintf(file, "\\vspace{0.5em}\n");

    return TREE_ERROR_NO;
}


tree_error_type dump_derivative_to_file(FILE* file, tree_t* derivative_tree, double derivative_result, int derivative_order)
{
    if (file == NULL || derivative_tree == NULL)
        return TREE_ERROR_NULL_PTR;

    char derivative_expr[MAX_LENGTH_OF_TEX_EXPRESSION] = {0};
    int position = 0;
    tree_to_string_simple(derivative_tree -> root, derivative_expr, &position, sizeof(derivative_expr));

    const char* derivative_notation = NULL;
    char custom_notation[32] = {0};

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
    fprintf(file, "Derivative: \\[ %s = %s \\]\n\n", derivative_notation, derivative_expr);
    fprintf(file, "Value of derivative at point: \\[ %s = %.6f \\]\n\n", derivative_notation, derivative_result);

    return TREE_ERROR_NO;
}


tree_error_type dump_variable_table_to_file(FILE* file, variable_table* var_table)
{
    if (file == NULL || var_table == NULL)
        return TREE_ERROR_NULL_PTR;

    if (var_table -> number_of_variables <= 0)
        return TREE_ERROR_NO;

    fprintf(file, "\\section*{Variable Table}\n");
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
