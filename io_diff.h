#ifndef IO_DIFFERENCIATOR_H_
#define IO_DIFFERENCIATOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "tree_base.h"
#include "tree_error_types.h"

const int COEFFICIENT = 2;

void init_load_progress(load_progress* progress);
void add_node_to_load_progress(load_progress* progress, node_t* node, size_t depth);
void free_load_progress(load_progress* progress);

size_t get_file_size(FILE *file);
node_type determine_node_type(const char* token);
void skip_spaces(const char* buffer, size_t* position);
char* read_token(char* buffer, size_t* position);
node_t* read_node_from_buffer(tree_t* tree, char* buffer, size_t buffer_length, size_t* position,
                              node_t* parent, int depth, load_progress* progress);
tree_error_type tree_load(tree_t* tree, const char* filename);


#endif //IO_DIFFERENCIATOR_H_
