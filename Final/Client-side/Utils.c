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

int open_writeToFile(const char *path, const char *buf ,const size_t buf_size){
    int fd = open(path, O_RDONLY);

    if (fd < 1) {
        int fd = open(path, O_WRONLY);

        if (fd < 1) {
            fprintf(stderr, "Error: Failed to open %s\n", path);
            return -1;
        }

        write(fd, buf, buf_size);
    } else {
        read(fd, buf, buf_size);
    }

    close(fd);
    return 0;
}