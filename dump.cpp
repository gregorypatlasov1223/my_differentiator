#include <time.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "dump.h"
#include "tree_base.h"
#include "tree_common.h"
#include "tree_error_types.h"


const char* get_node_type_string(node_type type)
{
    switch(type)
    {
        case NODE_OP:  return "OP";
        case NODE_NUM: return "NUM";
        case NODE_VAR: return "VAR";
        default:       return "UNKNOWN";
    }
}


const char* node_data_to_string(const node_t* node, char* buffer, size_t buffer_size)
{
    if (node == NULL) return "NULL";

    switch(node -> type)
    {
        case NODE_OP:
            switch(node -> data.op_value)
            {
                case OP_ADD: return "+";
                case OP_SUB: return "-";
                case OP_MUL: return "*";
                case OP_DIV: return "/";
                case OP_SIN: return "sin";
                case OP_COS: return "cos";
                default: return "UNKNOWN_OP";
            }

        case NODE_NUM:
            snprintf(buffer, buffer_size, "%.2f", node -> data.num_value);
            return buffer;

        case NODE_VAR:
            if (node->data.var_definition.name != NULL)
                snprintf(buffer, buffer_size, "%s", node->data.var_definition.name);
            else
                snprintf(buffer, buffer_size, "var_%zu", node->data.var_definition.hash);
            
            return buffer;

        default:
            return "UNKNOWN_TYPE_OF_NODE";
    }
}


// ============================COMMON_DUMP===========================================


tree_error_type print_tree_node(const node_t* node)
{
    if (node == NULL)
    {
        printf("()");
        return TREE_ERROR_NO;
    }

    char buffer[MAX_SIZE_OF_BUFFER] ={};
    printf("%s", node_data_to_string(node, buffer, sizeof(buffer)));

    printf("(");
    if (node -> left)
        print_tree_node(node -> left);

    if (node -> right)
        print_tree_node(node -> right);

    printf(")");

    return TREE_ERROR_NO;
}


tree_error_type tree_common_dump(tree_t* tree)
{
    if (tree == NULL)
    {
        printf("Tree is NULL");
        return TREE_ERROR_NULL_PTR;
    }

    printf("TREE DUMP\n");
    printf("Tree size: %lu\n", tree -> size);
    printf("Tree structure:\n");

    if (tree -> root == NULL)
        printf("EMPTY TREE");
    else
        print_tree_node(tree -> root);

    putchar('\n');
    return TREE_ERROR_NO;
}


// ============================GRAPHIC_DUMP===========================================


tree_error_type generate_dot_file(tree_t* tree, const char* filename)
{
    assert(tree);
    assert(filename);

    FILE* dot_file = fopen(filename, "w");
    if (!dot_file)
        return TREE_ERROR_OPENING_FILE;

    fprintf(dot_file, "digraph BinaryTree {\n");
    fprintf(dot_file, "    rankdir=TB;\n");
    fprintf(dot_file, "    node [shape=record, style=filled, color=black];\n\n");

    create_dot_nodes(tree, dot_file);

    if (tree -> root != NULL)
        create_tree_connections(tree -> root, dot_file);
    else
        fprintf(dot_file, "    empty [label=\"Empty Tree\", shape=plaintext];\n");

    fprintf(dot_file, "}\n");
    fclose(dot_file);

    return TREE_ERROR_NO;
}


void create_node_recursive(node_t* node, tree_t* tree, FILE* dot_file)
{
    if (node == NULL)
        return;

    const char* color = get_node_color(node, tree);
    const char* shape = "record"; // форма по умолчанию

    char buffer[MAX_SIZE_OF_BUFFER] = {};
    const char* node_data = node_data_to_string(node, buffer, sizeof(buffer));

    if (node -> type == NODE_VAR)
    {
        shape = "ellipse";
        // для эллипсов используем простой текст
        fprintf(dot_file, "    node_%p [label=\"%s\\naddress: %p\\nleft: %p\\nright: %p\\nparent: %p\", fillcolor=%s, shape=%s];\n",
                (void*)node, node_data, (void*)node,
                (void*)node -> left, (void*)node -> right, (void*)node -> parent, color, shape);
    }
    else
    {
        shape = "record";
        // для остальных используем record-синтаксис
        fprintf(dot_file, "    node_%p [label=\"{ {data: %s} | {address: %p} | {left %p| right %p| parent %p} }\", fillcolor=%s, shape=%s];\n",
                (void*)node, node_data, (void*)node,
                (void*)node -> left, (void*)node -> right, (void*)node -> parent, color, shape);
    }

    create_node_recursive(node -> left,  tree, dot_file);
    create_node_recursive(node -> right, tree, dot_file);
}


void create_dot_nodes(tree_t* tree, FILE* dot_file)
{
    assert(tree);
    assert(dot_file);

    create_node_recursive(tree -> root, tree, dot_file);
}


void create_tree_connections(node_t* node, FILE* dot_file)
{
    if (node == NULL)
        return;

    if (node -> left != NULL)
    {
        if (node -> left -> parent == node)
        {
            fprintf(dot_file, "    node_%p -> node_%p [color=blue, dir=both, arrowtail=normal, arrowhead=normal, label=\"L\"];\n",
                    (void*)node, (void*)node -> left);
        }
        else
        {
            fprintf(dot_file, "    node_%p -> node_%p [color=blue, label=\"L\"];\n",
                    (void*)node, (void*)node -> left);

            fprintf(dot_file, "    error_parent_%p [shape=ellipse, style=filled, fillcolor=orange, label=\"Parent address Error\"];\n",
                    (void*)node -> left);
            fprintf(dot_file, "    node_%p -> error_parent_%p [color=red];\n", (void*)node -> left, (void*)node -> left);
        }
        create_tree_connections(node -> left, dot_file);
    }

    if (node -> right != NULL)
    {
        if (node -> right -> parent == node)
        {
            fprintf(dot_file, "    node_%p -> node_%p [color=green, dir=both, arrowtail=normal, arrowhead=normal, label=\"R\"];\n",
                    (void*)node, (void*)node -> right);
        }
        else
        {
            fprintf(dot_file, "    node_%p -> node_%p [color=green, label=\"R\"];\n",
                    (void*)node, (void*)node -> right);

            fprintf(dot_file, "    error_parent_%p [shape=ellipse, style=filled, fillcolor=orange, label=\"Parent address Error\"];\n",
                    (void*)node -> right);
            fprintf(dot_file, "    node_%p -> error_parent_%p [color=red, style=dashed];\n", (void*)node -> right, (void*)node -> right);
        }
        create_tree_connections(node -> right, dot_file);
    }
}


const char* get_node_color(node_t* node, tree_t* tree)
{
    if (node == tree -> root)
        return "lightcoral";
    else if (node -> type == NODE_VAR)
        return "lightgreen";  // VAR - зеленый
    else if (node -> type == NODE_NUM)
        return "lightyellow"; // NUM - желтый
    else if (node -> type == NODE_OP)
    {
        // Разные цвета для разных операторов
        switch (node -> data.op_value)
        {
            case OP_ADD: return "lightblue";     // + - голубой
            case OP_SUB: return "lightpink";     // - - розовый
            case OP_MUL: return "lightsalmon";   // * - оранжевый
            case OP_DIV: return "plum";          // / - фиолетовый
            case OP_SIN: return "lightseagreen";
            case OP_COS: return "mediumpurple";
            default:     return "lightgrey";
        }
    }
    else
        return "lightblue";
}


void write_highlighted_buffer(FILE* htm_file, const char* buffer, size_t buffer_length, size_t position)
{
    if (buffer == NULL || htm_file == NULL)
        return;

    fprintf(htm_file, "<div style='margin:10px 0; padding:10px; background:#f5f5f5; border:1px solid #ddd; font-family:monospace; font-size:14px;'>\n");
    fprintf(htm_file, "<p style='margin:0 0 5px 0; font-weight:bold;'>Текущая позиция в буфере: %lu</p>\n", position);
    fprintf(htm_file, "<div style='background:white; padding:8px; border:1px solid #ccc; word-wrap:break-word;'>\n");

    if (position > 0)
    {
        fprintf(htm_file, "<span style='color:#666;'>");
        for (size_t i = 0; i < position; i++)
        {
            if (buffer[i] != '\0')
                fputc(buffer[i], htm_file);
            else
                fputc('"', htm_file);
        }
        fprintf(htm_file, "</span>");
    }
    printf("buffer_length: %lu position:%lu\n", buffer_length, position);

    // текущий символ красный цвет с подсветкой
    if (position < buffer_length)
        fprintf(htm_file, "<span style='color:red; font-weight:bold; background:#ffe6e6; padding:1px 3px; border:1px solid #ff9999; border-radius:2px;'>%c</span>",
                buffer[position]);
    else
        fprintf(htm_file, "<span style='color:red; font-weight:bold; background:#ffe6e6; padding:1px 3px; border:1px solid #ff9999; border-radius:2px;'>КОНЕЦ</span>");

    // часть после текущей позиции - синий цвет
    if (position + 1 < buffer_length)
    {
        fprintf(htm_file, "<span style='color:#0066cc;'>");
        for (size_t i = position + 1; i < buffer_length; i++)
        {
            if (buffer[i] != '\0')
                fputc(buffer[i], htm_file);
            else
                fputc('"', htm_file);
        }
        fprintf(htm_file, "</span>");
    }

    fprintf(htm_file, "</div>\n");

    fprintf(htm_file, "</div>\n");
}


void write_tree_info(FILE* htm_file, tree_t* tree, const char* buffer, size_t buffer_length, size_t buffer_position)
{
    assert(htm_file);

    fprintf(htm_file, "<div style='margin-bottom:15px;'>\n");

    if (tree != NULL)
    {
        fprintf(htm_file, "<p><b>Размер дерева:</b> %lu</p>\n", tree -> size);

        if (tree -> root != NULL)
        {
            char root_buffer[MAX_SIZE_OF_BUFFER] = {};
            const char* root_data_str = node_data_to_string(tree -> root, root_buffer, sizeof(root_buffer));

            fprintf(htm_file, "<p><b>Адрес корня:</b> %p</p>\n", (void*)tree -> root);
            fprintf(htm_file, "<p><b>Данные корня:</b> %s</p>\n", root_data_str);
        }
        else
        {
            fprintf(htm_file, "<p><b>Корень:</b> NULL</p>\n");
        }
    }
    else
    {
        fprintf(htm_file, "<p><b>Дерево:</b> NULL (парсинг в процессе)</p>\n");
    }

    if (buffer != NULL)
       write_highlighted_buffer(htm_file, buffer, buffer_length, buffer_position);

    if (tree != NULL)
    {
        tree_verify_result verify_result = verify_tree(tree);
        const char* verify_result_in_string = tree_verify_result_to_string(verify_result);
        const char* verify_color = (verify_result == TREE_VERIFY_SUCCESS) ? "green" : "red";

        fprintf(htm_file, "<p><b>Результат проверки:</b> <span style='color:%s; font-weight: bold;'>%s</span></p>\n",
                verify_color, verify_result_in_string);
    }
    else
    {
        fprintf(htm_file, "<p><b>Результат проверки:</b> <span style='color:gray; font-weight: bold;'>N/A (парсинг)</span></p>\n");
    }

    fprintf(htm_file, "</div>\n");
}


tree_error_type write_tree_common_picture(tree_t* tree, FILE* htm_file, const char* folder_path, const char* folder_name)
{
    static int n_of_pictures = 0;

    char temp_dot_global_path[MAX_LENGTH_OF_FILENAME] = {};
    char temp_svg_global_path[MAX_LENGTH_OF_FILENAME] = {};

    snprintf(temp_dot_global_path, sizeof(temp_dot_global_path), "%s/tree_%d.dot",
             folder_path, n_of_pictures);
    snprintf(temp_svg_global_path, sizeof(temp_svg_global_path), "%s/tree_%d.svg",
             folder_path, n_of_pictures);

    char temp_svg_local_path[MAX_LENGTH_OF_FILENAME] = {};
    snprintf(temp_svg_local_path, sizeof(temp_svg_local_path), "%s/tree_%d.svg",
             folder_name, n_of_pictures);

    n_of_pictures++;

    tree_error_type dot_result = generate_dot_file(tree, temp_dot_global_path);
    if (dot_result != TREE_ERROR_NO)
        return dot_result;

    char command[MAX_LENGTH_OF_SYSTEM_COMMAND] = {};
    snprintf(command, sizeof(command), "dot -Tsvg -Gcharset=utf8 \"%s\" -o \"%s\"",
             temp_dot_global_path, temp_svg_global_path);

    int result = system(command);

    if (result == 0) // по-моему так нельзя
    {
        fprintf(htm_file, "<div style='text-align:center;'>\n");
        fprintf(htm_file, "<img src='%s' style='max-width:100%%; border:1px solid #ddd;'>\n", temp_svg_local_path);
        fprintf(htm_file, "</div>\n");
    }
    else
    {
        fprintf(htm_file, "<p style='color:red;'>Error generating SVG graph</p>\n");
    }

    remove(temp_dot_global_path);

    return TREE_ERROR_NO;
}


void write_dump_header(FILE* htm_file, time_t now, const char* comment)
{
    assert(htm_file);

    fprintf(htm_file, "<div style='border:2px solid #ccc; margin:10px; padding:15px; background:#f9f9f9;'>\n");
    fprintf(htm_file, "<h2 style='color:#333;'>Tree Dump at %s</h2>\n", ctime(&now));

    if (comment != NULL)
    {
        fprintf(htm_file, "<div style='background: #2d2d2d; padding: 10px; margin: 10px 0; border-left: 4px solid #ff6b6b;'>\n");
        fprintf(htm_file, "<p style='color: #ff6b6b; font-weight:bold; margin:0;'>%s</p>\n", comment);
        fprintf(htm_file, "</div>\n");
    }
}


void write_dump_footer(FILE* htm_file)
{
    assert(htm_file);
    fprintf(htm_file, "</div>\n\n");
}


tree_error_type tree_dump_to_htm(tree_t* tree, FILE* htm_file, const char* folder_path, const char* folder_name, const char* comment)
{
    assert(htm_file);
    assert(folder_path);

    time_t now = time(NULL);

    write_dump_header(htm_file, now, comment);
    write_tree_info(htm_file, tree, NULL, 0, 0);

    if (tree != NULL && tree->root != NULL)
    {
        tree_error_type result_writing_picture = write_tree_common_picture(tree, htm_file, folder_path, folder_name);
        if (result_writing_picture != TREE_ERROR_NO)
            return result_writing_picture;
    }
    else
    {
        fprintf(htm_file, "<p style='color:blue;'>No tree to visualize</p>\n");
    }

    write_dump_footer(htm_file);
    return TREE_ERROR_NO;
}


tree_error_type generate_load_progress_dot_file(tree_t* tree, load_progress* progress, const char* filename)
{
    assert(progress);
    assert(filename);

    FILE* dot_file = fopen(filename, "w");
    if (!dot_file)
        return TREE_ERROR_OPENING_FILE;

    fprintf(dot_file, "digraph LoadProcess {\n");
    fprintf(dot_file, "    rankdir=TB;\n");
    fprintf(dot_file, "    node [style=filled, color=black];\n\n");

    for (size_t i = 0; i < progress->size; i++)
    {
        node_t* node   = progress -> items[i].node;
        size_t depth = progress -> items[i].depth;
        const char* color = get_node_color(node, tree);
        char buffer[MAX_SIZE_OF_BUFFER];
        const char* node_data = node_data_to_string(node, buffer, sizeof(buffer));
        const char* shape = (node -> type == NODE_VAR) ? "ellipse" : "record";

        if (node -> type == NODE_VAR)
        {
            // Для эллипсов используем простой текст
            fprintf(dot_file, "    node_%p [label=\"%s\\n Глубина: %zu\", fillcolor=%s, shape=%s];\n",
                    (void*)node, node_data, depth, color, shape);
        }
        else
        {
            // для остальных используем record-синтаксис
            fprintf(dot_file, "    node_%p [label=\"{{%s}|{Глубина: %zu}}\", fillcolor=%s, shape=%s];\n",
                    (void*)node, node_data, depth, color, shape);
        }
    }

    size_t max_depth = 0;
    for (size_t i = 0; i < progress -> size; i++)
    {
        if (progress -> items[i].depth > max_depth)
            max_depth = progress -> items[i].depth;
    }

    for (size_t depth = 0; depth <= max_depth; depth++)
    {
        fprintf(dot_file, "    { rank = same; ");
        for (size_t i = 0; i < progress -> size; i++)
        {
            if (progress -> items[i].depth == depth)
                fprintf(dot_file, "node_%p; ", (void*)progress -> items[i].node);
        }
        fprintf(dot_file, "}\n");
    }

    for (size_t i = 0; i < progress->size; i++)
    {
        node_t* node = progress -> items[i].node;
        if (node -> left)
        {
            fprintf(dot_file, "    node_%p -> node_%p [color=blue, label=\"L\"];\n",
                    (void*)node, (void*)node -> left);
        }
        if (node -> right)
        {
            fprintf(dot_file, "    node_%p -> node_%p [color=green, label=\"R\"];\n",
                    (void*)node, (void*)node -> right);
        }
    }

    fprintf(dot_file, "}\n");
    fclose(dot_file);

    return TREE_ERROR_NO;
}


tree_error_type tree_load_dump_to_htm(tree_t* tree, FILE* htm_file, const char* folder_path, const char* folder_name,
                               const char* buffer, size_t buffer_length, size_t buffer_position, load_progress* progress, const char* comment)
{
    assert(htm_file);
    assert(folder_path);

    time_t now = time(NULL);

    write_dump_header(htm_file, now, comment);
    write_tree_info(htm_file, tree, buffer, buffer_length, buffer_position);

    if (progress != NULL && progress -> size > 0)
    {
        static int n_of_pictures = 0;

        char temp_dot_global_path[MAX_LENGTH_OF_FILENAME] = {};
        char temp_svg_global_path[MAX_LENGTH_OF_FILENAME] = {};
        snprintf(temp_dot_global_path, sizeof(temp_dot_global_path), "%s/load_%d.dot",
                 folder_path, n_of_pictures);
        snprintf(temp_svg_global_path, sizeof(temp_svg_global_path), "%s/load_%d.svg",
                 folder_path, n_of_pictures);

        char temp_svg_local_path[MAX_LENGTH_OF_FILENAME] = {};
        snprintf(temp_svg_local_path, sizeof(temp_svg_local_path), "%s/load_%d.svg",
                 folder_name, n_of_pictures);

        n_of_pictures++;

        tree_error_type dot_result = generate_load_progress_dot_file(tree, progress, temp_dot_global_path);
        if (dot_result != TREE_ERROR_NO)
            return dot_result;

        char svg_command[MAX_LENGTH_OF_SYSTEM_COMMAND] = {};
        snprintf(svg_command, sizeof(svg_command), "dot -Tsvg -Gcharset=utf8 \"%s\" -o \"%s\"",
                 temp_dot_global_path, temp_svg_global_path);

        int result = system(svg_command);

        if (result == 0) // по-моему так нельзя
        {
            fprintf(htm_file, "<div style='text-align:center;'>\n");
            fprintf(htm_file, "<img src='%s' style='max-width:100%%; border:1px solid #ddd;'>\n", temp_svg_local_path);
            fprintf(htm_file, "</div>\n");
        }
        else
        {
            fprintf(htm_file, "<p style='color:red;'>Error generating SVG graph</p>\n");
        }

        remove(temp_dot_global_path);
    }
    else if (tree != NULL && tree -> root != NULL)
    {
        tree_error_type result_writing_picture = write_tree_common_picture(tree, htm_file, folder_path, folder_name);
        if (result_writing_picture != TREE_ERROR_NO)
            return result_writing_picture;
    }
    else
    {
        fprintf(htm_file, "<p style='color:blue;'>No tree to visualize (parsing in progress)</p>\n");
    }

    write_dump_footer(htm_file);
    return TREE_ERROR_NO;
}


tree_error_type tree_dump(tree_t* tree, const char* filename)
{
    if (tree == NULL)
        return TREE_ERROR_NULL_PTR;

    char command[MAX_LENGTH_OF_SYSTEM_COMMAND] = {};
    snprintf(command, sizeof(command), "mkdir -p %s", GENERAL_FOLDER_NAME_FOR_LOGS);
    system(command);

    char folder_name[MAX_LENGTH_OF_FILENAME] = {};
    snprintf(folder_name, sizeof(folder_name), "%s_dump", filename);

    char folder_path[MAX_LENGTH_OF_FILENAME] = {};
    snprintf(folder_path, sizeof(folder_path), "%s/%s_dump", GENERAL_FOLDER_NAME_FOR_LOGS, filename);

    snprintf(command, sizeof(command), "mkdir -p %s", folder_path);
    system(command);

    char htm_filename[MAX_LENGTH_OF_FILENAME] = {};
    snprintf(htm_filename, sizeof(htm_filename), "%s/%s.htm", GENERAL_FOLDER_NAME_FOR_LOGS, filename);

    FILE* htm_file = fopen(htm_filename, "a");
    if (!htm_file)
        return TREE_ERROR_OPENING_FILE;

    tree_error_type result = tree_dump_to_htm(tree, htm_file, folder_path, folder_name, NULL);

    fclose(htm_file);
    return result;
}


tree_error_type tree_load_dump(tree_t* tree, const char* filename, const char* buffer, size_t buffer_length,
                           size_t buffer_position, load_progress* progress, const char* comment)
{
    if (tree == NULL && progress == NULL)
        return TREE_ERROR_NULL_PTR;

    char command[MAX_LENGTH_OF_SYSTEM_COMMAND] = {};
    snprintf(command, sizeof(command), "mkdir -p %s", GENERAL_FOLDER_NAME_FOR_LOGS);
    system(command);

    char folder_name[MAX_LENGTH_OF_FILENAME] = {};
    snprintf(folder_name, sizeof(folder_name), "%s_dump", filename);

    char folder_path[MAX_LENGTH_OF_FILENAME] = {};
    snprintf(folder_path, sizeof(folder_path), "%s/%s_dump", GENERAL_FOLDER_NAME_FOR_LOGS, filename);

    snprintf(command, sizeof(command), "mkdir -p %s", folder_path);
    system(command);

    char htm_filename[MAX_LENGTH_OF_FILENAME] = {};
    snprintf(htm_filename, sizeof(htm_filename), "%s/%s.htm", GENERAL_FOLDER_NAME_FOR_LOGS, filename);

    FILE* htm_file = fopen(htm_filename, "a");
    if (!htm_file)
        return TREE_ERROR_OPENING_FILE;

    tree_error_type result = tree_load_dump_to_htm(tree, htm_file, folder_path, folder_name, buffer, buffer_length, buffer_position, progress, comment);

    fclose(htm_file);
    return result;
}


tree_error_type init_tree_log(const char* filename)
{
    assert(filename);

    char htm_filename[MAX_LENGTH_OF_FILENAME] = {};
    snprintf(htm_filename, sizeof(htm_filename), "%s/%s.htm", GENERAL_FOLDER_NAME_FOR_LOGS, filename);

    FILE* htm_file = fopen(htm_filename, "w");
    if (!htm_file)
        return TREE_ERROR_OPENING_FILE;

    fprintf(htm_file, "<!DOCTYPE html>\n"
                      "<html>\n"
                      "<head>\n"
                      "<title>Tree Dump Log</title>\n"
                      "<meta charset='UTF-8'>\n"
                      "<style>\n"
                      "body { font-family: Arial, sans-serif; margin: 20px; }\n"
                      "table { border-collapse: collapse; width: 100%%; }\n"
                      "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }\n"
                      "th { background-color: #f2f2f2; }\n"
                      "</style>\n"
                      "</head>\n"
                      "<body>\n"
                      "<h1>Tree Dump Log</h1>\n");
    fclose(htm_file);

    return TREE_ERROR_NO;
}


tree_error_type close_tree_log(const char* filename)
{
    assert(filename);

    char htm_filename[MAX_LENGTH_OF_FILENAME] = {};
    snprintf(htm_filename, sizeof(htm_filename), "%s/%s.htm", GENERAL_FOLDER_NAME_FOR_LOGS, filename);

    FILE* htm_file = fopen(htm_filename, "a");
    if (!htm_file)
        return TREE_ERROR_OPENING_FILE;

    fprintf(htm_file, "</body>\n");
    fprintf(htm_file, "</html>\n");
    fclose(htm_file);

    return TREE_ERROR_NO;
}


tree_verify_result verify_tree(tree_t* tree)
{
    if (tree == NULL)
        return TREE_VERIFY_NULL_PTR;

    if (tree -> root != NULL && tree -> root -> parent != NULL)
        return TREE_VERIFY_INVALID_PARENT;

    return TREE_VERIFY_SUCCESS;
}


const char* tree_verify_result_to_string(tree_verify_result result)
{
    switch (result)
    {
        case TREE_VERIFY_SUCCESS:          return "Success";
        case TREE_VERIFY_NULL_PTR:         return "Null pointer";
        case TREE_VERIFY_INVALID_PARENT:   return "Invalid parent pointer";
        case TREE_VERIFY_CYCLE_DETECTED:   return "Cycle detected";
        default:                           return "Unknown error";
    }
}
