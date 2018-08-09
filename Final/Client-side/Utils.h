#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

int string_split(char *input, char **argument, const char delimiter);

void write_buffer(char **dest_ptr, char *src, size_t size);

int open_writeToFile(const char *path, const char *buf ,const size_t buf_size);