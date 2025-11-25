#include <stdio.h>
#include <stdlib.h>

#include "dump.h"
#include "io_diff.h"
#include "tree_base.h"
#include "operations.h"
#include "latex_dump.h"
#include "tree_common.h"
#include "variable_parse.h"
#include "user_interface.h"

int main()
{
    tree_t tree = {};
    tree_constructor(&tree);

    variable_table var_table = {};
    init_variable_table(&var_table);

    double result = 0.0;

    init_tree_log("differenciator_tree");
    init_tree_log("differentiator_parse");

    tree_error_type error = tree_load(&tree, "differenciator_tree.txt");

    if (error == TREE_ERROR_NO)
    {
        printf("The tree has been uploaded successfully!\n");
        tree_common_dump(&tree);

        printf("\nCALCULATING THE EXPRESSION\n");
        tree_error_type mistake = evaluate_tree(&tree, &var_table, &result);

        if (mistake == TREE_ERROR_NO)
        {
            printf("Calculation result: %.6f\n", result);
        }
        else
        {
            printf("Calculation error: %d\n", mistake);
            print_tree_error(mistake);
        }

        printf("\nCALCULATING DERIVATIVES\n");

        tree_t derivative_trees[MAX_NUMBER_OF_DERIVATIVE]   = {};
        double derivative_results[MAX_NUMBER_OF_DERIVATIVE] = {};
        int    actual_derivative_count = 0;

        tree_t* current_tree = &tree;
        for (int i = 0; i < MAX_NUMBER_OF_DERIVATIVE; i++)
        {
            tree_constructor(&derivative_trees[i]);

            error = differentiate_tree(current_tree, "x", &derivative_trees[i]);
            if (error != TREE_ERROR_NO)
            {
                printf("Error in calculating the derivative of the order %d: %d\n", i + 1, error);
                tree_destructor(&derivative_trees[i]);
                break;
            }

            printf("The derivative of order %d has been successfully calculated!\n", i + 1);
            tree_common_dump(&derivative_trees[i]);

            error = evaluate_tree(&derivative_trees[i], &var_table, &derivative_results[i]);
            if (error == TREE_ERROR_NO)
            {
                printf("The value of the derivative of the order %d: %.6f\n", i + 1, derivative_results[i]);
                actual_derivative_count++;
            }
            else
            {
                printf("Error in calculating the value of the derivative of the order %d\n", i + 1);
                print_tree_error(error);
                derivative_results[i] = 0.0;
                actual_derivative_count++;
            }

            current_tree = &derivative_trees[i];
        }

        printf("\nLATEX DOCUMENT GENERATION\n");

        if (actual_derivative_count > 0)
        {
            tree_t** derivative_trees_ptr = (tree_t**)calloc(actual_derivative_count, sizeof(tree_t*)); //массив указателей на деревья производных
            for (int i = 0; i < actual_derivative_count; i++)
                derivative_trees_ptr[i] = &derivative_trees[i];

            error = generate_latex_dump_with_derivatives(&tree, derivative_trees_ptr, derivative_results,
                                                   actual_derivative_count, &var_table,
                                                   "expression_analysis.tex", result);

            free(derivative_trees_ptr);
        }

        if (error == TREE_ERROR_NO)
        {
            printf("The LaTeX document has been created successfully!\n");
            if (actual_derivative_count > 0)
            {
                printf("The document contains derivatives up to %d order\n", actual_derivative_count);
            }
        }
        else
        {
            printf("Error creating a LaTeX document: %d\n", error);
        }

        for (int i = 0; i < actual_derivative_count; i++)
        {
            tree_destructor(&derivative_trees[i]);
        }
    }
    else
    {
        printf("Error loading the tree: %d\n", error);
        print_tree_error(error);
    }


    close_tree_log("differenciator_tree");
    close_tree_log("differentiator_parse");

    destroy_variable_table(&var_table);
    tree_destructor(&tree);

    return 0;
}
