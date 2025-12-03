#include <stdio.h>
#include <stdlib.h>

#include "dump.h"
#include "user_interface.h"
#include "processing_diff.h"
#include "tree_error_types.h"


int main(int argc, const char** argv)
{
    differentiator_struct* diff_struct = create_differentiator_struct();
    if (!diff_struct)
    {
        fprintf(stderr, "Critical error: Failed to create a differentiator structure\n");
        return 1;
    }

    init_tree_log("differenciator_tree");
    init_tree_log("differentiator_parse");

    tree_error_type error = TREE_ERROR_NO;

    if (error == TREE_ERROR_NO) error = initialize_expression(diff_struct, argc, argv);

    if (error == TREE_ERROR_NO) error = parse_expression_tree(diff_struct);

    if (error == TREE_ERROR_NO) error = initialize_latex_output(diff_struct);

    if (error == TREE_ERROR_NO) error = request_variable_values(diff_struct);

    if (error == TREE_ERROR_NO) error = evaluate_original_function(diff_struct);

    if (error == TREE_ERROR_NO) error = optimize_expression_tree(diff_struct);

    // строим график (если возможно)
    if (error == TREE_ERROR_NO)
    {
        tree_error_type plot_error = plot_function_graph(diff_struct);
        if (plot_error != TREE_ERROR_NO && plot_error != TREE_ERROR_NO_VARIABLES)
        {
            fprintf(stderr, "Failed to build a graph: %s\n", tree_error_translator(plot_error));
        }
    }

    if (error == TREE_ERROR_NO) error = perform_differentiation_process(diff_struct);

    if (error == TREE_ERROR_NO && diff_struct -> tex_file)
    {
        error = finalize_latex_output(diff_struct);
    }

    close_tree_log("differenciator_tree");
    close_tree_log("differentiator_parse");

    if (error == TREE_ERROR_NO)
    {
        printf("\n The program has been successfully completed!\n");
    }
    else
    {
        fprintf(stderr, "\n The program terminated with an error:\n");
        fprintf(stderr, "  Error code: %d\n", error);
        fprintf(stderr, "  Description: %s\n", tree_error_translator(error));
    }

    destroy_differentiator_struct(diff_struct);

    return (error == TREE_ERROR_NO) ? 0 : 1;
}
