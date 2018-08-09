#include "Utils.h"

int string_split(char *input, char **argument, const char delimiter)
{
    char *position = strchr(input, delimiter);

    if (position == NULL)
        return 0;

    int index = (position - input) / sizeof(char);
    *argument = position + sizeof(char);
    input[index] = '\0';
    return 1;
}

void write_buffer(char **dest_ptr, char *src, size_t size){
    memcpy(*dest_ptr, src, size);
    *dest_ptr += size;
}