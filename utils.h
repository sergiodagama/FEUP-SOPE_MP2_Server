#ifndef UTILS_H
#define UTILS_H
#include "queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

void input_error();

bool str_cmp(char *string1, char *string2);

bool is_number(char *str);

void free_memory(queue* trash);

#endif