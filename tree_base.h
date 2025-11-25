#ifndef DIFFERENTIATOR_H_
#define DIFFERENTIATOR_H_

#include <stddef.h>
#include <stdio.h>
#include <time.h>

#include "tree_common.h"
#include "tree_error_types.h"

#define MAX_LENGTH_OF_ADDRESS 128
#define MAX_SIZE_OF_BUFFER 512
#define MAX_LENGTH_OF_ANSWER 1024
#define MAX_PATH_DEPTH 512
#define MAX_LENGTH_OF_FILENAME 256
#define MAX_LENGTH_OF_SYSTEM_COMMAND 512
const char* const GENERAL_FOLDER_NAME_FOR_LOGS = "tree_logs";

#define COMPLETED_SUCCESSFULLY 1

#define BASE_EDGE_LENGTH 1.0
#define DEPTH_SPREAD_FACTOR 0.5
#define ZERO_RANK 0

bool is_leaf(node_t* node);
tree_error_type tree_destroy_recursive(node_t* node);
tree_error_type tree_constructor(tree_t* tree);
tree_error_type tree_destructor(tree_t* tree);
size_t compute_hash(const char* string);
void clear_input_buffer();




#endif // DIFFERENTIATOR_H_
