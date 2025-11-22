#include <stdio.h>
#include <stdlib.h>


#include "dump.h"
#include "io_diff.h"
#include "tree_base.h"
#include "operations.h"
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
    }

    close_tree_log("differenciator_tree");
    close_tree_log("differentiator_parse");

    destroy_variable_table(&var_table);
    tree_destructor(&tree);

    return 0;
}
