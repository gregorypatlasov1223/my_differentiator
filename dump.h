#ifndef DUMP_H_
#define DUMP_H_

#include <time.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "user_interface.h"

const char* get_node_type_string(node_type type);
const char* node_data_to_string(const node_t* node, char* buffer, size_t buffer_size);
tree_error_type print_tree_node(const node_t* node);
tree_error_type tree_common_dump(tree_t* tree);

// ============================GRAPHIC_DUMP===========================================

tree_error_type generate_dot_file(tree_t* tree, const char* filename);
void create_node_recursive(node_t* node, tree_t* tree, FILE* dot_file);
void create_dot_nodes(tree_t* tree, FILE* dot_file);
void process_child_node(node_t* parent, node_t* child, FILE* dot_file,
                       const char* color, const char* label, const char* error_style);
void create_tree_connections(node_t* node, FILE* dot_file);
const char* get_node_color(node_t* node, tree_t* tree);
void write_highlighted_buffer(FILE* htm_file, const char* buffer, size_t buffer_length, size_t position);
void write_tree_basic_info(FILE* htm_file, tree_t* tree);
void write_tree_verification_info(FILE* htm_file, tree_t* tree);
void write_tree_info(FILE* htm_file, tree_t* tree, const char* buffer, size_t buffer_length, size_t buffer_position);
tree_error_type write_tree_common_picture(tree_t* tree, FILE* htm_file, const char* folder_path, const char* folder_name);
void write_dump_header(FILE* htm_file, time_t now, const char* comment);
void write_dump_footer(FILE* htm_file);
tree_error_type tree_dump_to_htm(tree_t* tree, FILE* htm_file, const char* folder_path, const char* folder_name, const char* comment);
void write_dot_node(FILE* dot_file, node_t* node, size_t depth, const char* color, tree_t* tree);
void write_all_dot_nodes(FILE* dot_file, load_progress* progress, tree_t* tree);
size_t calculate_max_depth(load_progress* progress);
void write_depth_ranking(FILE* dot_file, load_progress* progress, size_t max_depth);
void write_node_connections(FILE* dot_file, load_progress* progress);
tree_error_type generate_load_progress_dot_file(tree_t* tree, load_progress* progress, const char* filename);
tree_error_type handle_load_progress(tree_t* tree, load_progress* progress, FILE* htm_file,
                                   const char* folder_path, const char* folder_name);
tree_error_type handle_regular_tree(tree_t* tree, FILE* htm_file,
                                  const char* folder_path, const char* folder_name);
tree_error_type tree_load_dump_to_htm(tree_t* tree, FILE* htm_file, const char* folder_path, const char* folder_name,
                               const char* buffer, size_t buffer_length, size_t buffer_position, load_progress* progress, const char* comment);
tree_error_type tree_dump(tree_t* tree, const char* filename);
tree_error_type tree_load_dump(tree_t* tree, const char* filename, const char* buffer, size_t buffer_length,
                           size_t buffer_position, load_progress* progress, const char* comment);
tree_error_type init_tree_log(const char* filename);
tree_error_type close_tree_log(const char* filename);
tree_verify_result verify_tree(tree_t* tree);
const char* tree_verify_result_to_string(tree_verify_result result);

#endif // DUMP_H_
