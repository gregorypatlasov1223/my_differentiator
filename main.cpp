#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dump.h"
#include "io_diff.h"
#include "tree_base.h"
#include "latex_dump.h"
#include "operations.h"
#include "tree_common.h"
#include "user_interface.h"
#include "variable_parse.h"

int main(int argc, const char** argv)
{
    const char* input_file = get_data_base_filename(argc, argv);

    tree_t tree = {};
    tree_constructor(&tree);

    variable_table var_table = {};
    init_variable_table(&var_table);

    double result = 0.0;

    init_tree_log("differenciator_tree");
    init_tree_log("differentiator_parse");

    tree_error_type error_loading = tree_load(&tree, input_file);

    FILE* tex_file = fopen("full_analysis.tex", "w");
    if (!tex_file)
    {
        printf("Error: failed to create file full_analysis.tex\n");
        return 1;
    }

    start_latex_dump(tex_file);

    if (error_loading == TREE_ERROR_NO)
    {
        tree_common_dump(&tree);

        for (int i = 0; i < var_table.number_of_variables; i++)
            request_variable_value(&var_table, var_table.variables[i].name);

        tree_error_type error = evaluate_tree(&tree, &var_table, &result);
        if (error == TREE_ERROR_NO)
            printf("Calculation result: %.6f\n", result);

        dump_original_function_to_file(tex_file, &tree, result);

        dump_variable_table_to_file(tex_file, &var_table);

        size_t size_before_optimization = count_tree_nodes(tree.root);

        error = optimize_tree_with_dump(&tree, tex_file, &var_table);

        if (error == TREE_ERROR_NO)
        {
            size_t size_after_optimization = count_tree_nodes(tree.root);
            printf("Optimization: %zu -> %zu nodes\n",
                   size_before_optimization, size_after_optimization);

            if (size_before_optimization != size_after_optimization)
            {
                error = evaluate_tree(&tree, &var_table, &result);
                if (error == TREE_ERROR_NO)
                    printf("Result after optimization: %.6f\n", result);
            }
        }

        fprintf(tex_file, "\\section*{Differentiation}\n");

        char* diff_variable = select_differentiation_variable(&var_table);
        if (diff_variable != NULL)
        {
            fprintf(tex_file, "Differentiation variable: \\[ %s \\]\n\n", diff_variable);

            tree_t   derivative_trees[MAX_NUMBER_OF_DERIVATIVE] = {};
            double derivative_results[MAX_NUMBER_OF_DERIVATIVE] = {};
            int    actual_derivative_count = 0;

            tree_t* current_tree = &tree;
            for (int i = 0; i < MAX_NUMBER_OF_DERIVATIVE; i++)
            {
                tree_constructor(&derivative_trees[i]);

                error = differentiate_tree(current_tree, diff_variable, &derivative_trees[i]);
                if (error != TREE_ERROR_NO)
                {
                    tree_destructor(&derivative_trees[i]);
                    break;
                }

                fprintf(tex_file, "\\subsection*{Optimization of derivative %d}\n", i + 1);
                error = optimize_tree_with_dump(&derivative_trees[i], tex_file, &var_table);

                error = evaluate_tree(&derivative_trees[i], &var_table, &derivative_results[i]);
                if (error == TREE_ERROR_NO)
                {
                    printf("Derivative %d: %.6f\n", i + 1, derivative_results[i]);
                    actual_derivative_count++;

                    dump_derivative_to_file(tex_file, &derivative_trees[i], derivative_results[i], i + 1);
                }

                current_tree = &derivative_trees[i];
                // if (i >= 2)
                //     break;
            }

            free(diff_variable);

            for (int i = 0; i < actual_derivative_count; i++)
            {
                tree_destructor(&derivative_trees[i]);
            }
        }
        else
        {
            fprintf(tex_file, "Failed to select variable for differentiation.\n\n");
        }
    }
    else
    {
        fprintf(tex_file, "\\section*{Error}\n");
        fprintf(tex_file, "Failed to load tree from file: %s\n", input_file);
    }

    end_latex_dump(tex_file);
    fclose(tex_file);

    printf("Full analysis saved to file: full_analysis.tex\n");
    printf("To compile: pdflatex full_analysis.tex\n");

    close_tree_log("differenciator_tree");
    close_tree_log("differentiator_parse");

    destroy_variable_table(&var_table);
    tree_destructor(&tree);

    return 0;
}
