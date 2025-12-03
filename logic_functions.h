#ifndef LOGIC_FUNCTIONS_H_
#define LOGIC_FUNCTIONS_H_

#include <stdbool.h>

#include "tree_common.h"

bool is_zero     (double number);
bool is_one      (double number);
bool is_minus_one(double number);
bool is_unary    (operation_type op);
bool is_binary   (operation_type op);

#endif // LOGIC_FUNCTIONS_H_
