#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dump.h"
#include "io_diff.h"
#include "new_input.h"
#include "latex_dump.h"
#include "operations.h"
#include "tree_common.h"
#include "user_interface.h"
#include "processing_diff.h"

// ==================== ФУНКЦИИ РАБОТЫ С КОНТЕКСТОМ ====================

differentiator_struct* create_differentiator_struct()
{
    differentiator_struct* diff_struct = (differentiator_struct*)calloc(1, sizeof(differentiator_struct));
    if (!diff_struct) return NULL;

    tree_constructor(&diff_struct -> tree);
    init_variable_table(&diff_struct -> var_table);

    return diff_struct;
}

void destroy_differentiator_struct(differentiator_struct* diff_struct)
{
    if (!diff_struct) return;

    if (diff_struct -> expression) free(diff_struct -> expression);
    if (diff_struct -> tex_file) fclose(diff_struct -> tex_file);

    destroy_variable_table(&diff_struct -> var_table);
    tree_destructor(&diff_struct -> tree);
    free(diff_struct);
}

// ==================== ОСНОВНЫЕ ФУНКЦИИ ОБРАБОТКИ ====================

tree_error_type initialize_expression(differentiator_struct* diff_struct, int argc, const char** argv)
{
    if (!diff_struct) return TREE_ERROR_NULL_PTR;

    const char* input_file = get_data_base_filename(argc, argv);
    if (!input_file)
    {
        return TREE_ERROR_NO_VARIABLES;
    }

    diff_struct -> expression = read_expression_from_file(input_file);
    if (!diff_struct -> expression)
    {
        return TREE_ERROR_OPENING_FILE;
    }

    printf("Expression from file: %s\n", diff_struct -> expression);
    return TREE_ERROR_NO;
}

tree_error_type parse_expression_tree(differentiator_struct* diff_struct)
{
    if (!diff_struct || !diff_struct -> expression) return TREE_ERROR_NULL_PTR;

    const char* ptr = diff_struct -> expression;
    diff_struct -> tree.root = get_G(&ptr, &diff_struct -> var_table);

    if (!diff_struct -> tree.root)
    {
        return TREE_ERROR_FORMAT;
    }

    diff_struct -> tree.size = count_tree_nodes(diff_struct -> tree.root);
    printf("Successfully parsed expression. Tree size: %zu\n", diff_struct -> tree.size);

    return TREE_ERROR_NO;
}

tree_error_type initialize_latex_output(differentiator_struct* diff_struct)
{
    if (!diff_struct) return TREE_ERROR_NULL_PTR;

    diff_struct -> tex_file = fopen(TEX_FILENAME, "w");
    if (!diff_struct -> tex_file)
    {
        return TREE_ERROR_OPENING_FILE;
    }

    start_latex_dump(diff_struct -> tex_file);
    return TREE_ERROR_NO;
}

tree_error_type request_variable_values(differentiator_struct* diff_struct)
{
    if (!diff_struct) return TREE_ERROR_NULL_PTR;

    for (int i = 0; i < diff_struct -> var_table.number_of_variables; i++)
    {
        tree_error_type error = request_variable_value(&diff_struct -> var_table, diff_struct -> var_table.variables[i].name);
        if (error != TREE_ERROR_NO)
        {
            return error;
        }
    }

    return TREE_ERROR_NO;
}

tree_error_type evaluate_original_function(differentiator_struct* diff_struct)
{
    if (!diff_struct) return TREE_ERROR_NULL_PTR;

    tree_error_type error = evaluate_tree(&diff_struct -> tree, &diff_struct -> var_table, &diff_struct -> result);
    if (error != TREE_ERROR_NO)
    {
        return error;
    }

    printf("Calculation result: %.6f\n", diff_struct -> result);

    if (diff_struct -> tex_file)
    {
        dump_original_function_to_file(diff_struct -> tex_file, &diff_struct -> tree, diff_struct -> result);
        dump_variable_table_to_file(diff_struct -> tex_file, &diff_struct -> var_table);
    }

    return TREE_ERROR_NO;
}

tree_error_type optimize_expression_tree(differentiator_struct* diff_struct)
{
    if (!diff_struct) return TREE_ERROR_NULL_PTR;

    size_t size_before = count_tree_nodes(diff_struct -> tree.root);

    tree_error_type error = optimize_tree_with_dump(&diff_struct -> tree, diff_struct -> tex_file, &diff_struct -> var_table);
    if (error != TREE_ERROR_NO)
    {
        return error;
    }

    size_t size_after = count_tree_nodes(diff_struct -> tree.root);
    printf("Optimization: %zu -> %zu nodes\n", size_before, size_after);

    if (size_before != size_after)
    {
        error = evaluate_tree(&diff_struct -> tree, &diff_struct -> var_table, &diff_struct -> result);
        if (error == TREE_ERROR_NO)
        {
            printf("Result after optimization: %.6f\n", diff_struct -> result);
        }
        return error;
    }

    return TREE_ERROR_NO;
}

tree_error_type plot_function_graph(differentiator_struct* diff_struct)
{
    if (!diff_struct || !diff_struct->tex_file) return TREE_ERROR_NULL_PTR;

    bool has_x_variable = false;
    double x_value = 0.0;

    for (int i = 0; i < diff_struct -> var_table.number_of_variables; i++)
    {
        if (strcmp(diff_struct -> var_table.variables[i].name, "x") == 0)
        {
            has_x_variable = true;
            x_value = diff_struct -> var_table.variables[i].value;
            break;
        }
    }

    if (!has_x_variable)
    {
        fprintf(diff_struct -> tex_file, "\\section*{Graph of the Function}\n");
        fprintf(diff_struct -> tex_file, "Cannot plot graph: variable 'x' not found in expression.\n");
        return TREE_ERROR_NO; // это не ошибка, просто нет переменной x
    }

    fprintf(diff_struct -> tex_file, "\\section*{Graph of the Function}\n");

    char latex_expr[MAX_LENGTH_OF_TEX_EXPRESSION] = {};
    int pos = 0;
    tree_to_string_simple(diff_struct -> tree.root, latex_expr, &pos, sizeof(latex_expr));

    char* pgfplot_expr = convert_latex_to_PGF_plot(latex_expr);
    if (!pgfplot_expr)
    {
        fprintf(diff_struct -> tex_file, "Failed to convert expression for plotting.\n");
        return TREE_ERROR_ALLOCATION;
    }

    printf("Plot expression: %s\n", pgfplot_expr);

    double x_min = x_value - 5.0;
    double x_max = x_value + 5.0;

    add_latex_plot(diff_struct->tex_file, pgfplot_expr, x_min, x_max, "Function Graph");
    free(pgfplot_expr);

    return TREE_ERROR_NO;
}

tree_error_type perform_differentiation_process(differentiator_struct* diff_struct)
{
    if (!diff_struct || !diff_struct->tex_file) return TREE_ERROR_NULL_PTR;

    fprintf(diff_struct -> tex_file, "\\section*{Differentiation}\n");

    char* diff_variable = select_differentiation_variable(&diff_struct -> var_table);
    if (!diff_variable)
    {
        fprintf(diff_struct -> tex_file, "Failed to select variable for differentiation.\n\n");
        return TREE_ERROR_NO_VARIABLES;
    }

    fprintf(diff_struct -> tex_file, "Differentiation variable: \\[ %s \\]\n\n", diff_variable);

    tree_t derivative_trees[MAX_NUMBER_OF_DERIVATIVE]   = {};
    double derivative_results[MAX_NUMBER_OF_DERIVATIVE] = {};
    int actual_derivative_count = 0;

    tree_t* current_tree = &diff_struct -> tree;

    for (int i = 0; i < MAX_NUMBER_OF_DERIVATIVE; i++)
    {
        tree_constructor(&derivative_trees[i]);

        tree_error_type error = differentiate_tree(current_tree, diff_variable, &derivative_trees[i]);
        if (error != TREE_ERROR_NO)
        {
            tree_destructor(&derivative_trees[i]);
            break;
        }

        fprintf(diff_struct -> tex_file, "\\subsection*{Optimization of derivative %d}\n", i + 1);
        error = optimize_tree_with_dump(&derivative_trees[i], diff_struct -> tex_file, &diff_struct -> var_table);

        error = evaluate_tree(&derivative_trees[i], &diff_struct -> var_table, &derivative_results[i]);
        if (error == TREE_ERROR_NO)
        {
            printf("Derivative %d: %.6f\n", i + 1, derivative_results[i]);
            actual_derivative_count++;

            dump_derivative_to_file(diff_struct -> tex_file, &derivative_trees[i], derivative_results[i], i + 1);
        }

        current_tree = &derivative_trees[i];
    }

    free(diff_variable);

    for (int i = 0; i < actual_derivative_count; i++)
    {
        tree_destructor(&derivative_trees[i]);
    }

    return TREE_ERROR_NO;
}

tree_error_type finalize_latex_output(differentiator_struct* diff_struct)
{
    if (!diff_struct) return TREE_ERROR_NULL_PTR;

    if (diff_struct -> tex_file)
    {
        end_latex_dump(diff_struct -> tex_file);
        fclose(diff_struct -> tex_file);
        diff_struct -> tex_file = NULL;

        printf("Full analysis saved to file: %s\n", TEX_FILENAME);
        printf("To compile: pdflatex %s\n", TEX_FILENAME);
    }

    return TREE_ERROR_NO;
}

void print_error_and_cleanup(differentiator_struct* diff_struct, tree_error_type error)
{
    if (error != TREE_ERROR_NO)
    {
        fprintf(stderr, "ERROR: %s\n", tree_error_translator(error));
    }

    if (diff_struct)
    {
        destroy_differentiator_struct(diff_struct);
    }
}
