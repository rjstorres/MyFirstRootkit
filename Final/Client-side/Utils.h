#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int string_split(char *input, char **argument, const char delimiter);

void write_buffer(char **dest_ptr, char *src, size_t size);