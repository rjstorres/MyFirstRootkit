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

void write_buffer(char **dest_ptr, char *src, size_t size)
{
    memcpy(*dest_ptr, src, size);
    *dest_ptr += size;
}

int open_writeToFile(const char *path, char *buf, const size_t buf_size)
{
    int fd = open(path, O_RDONLY);
    write(1, buf, buf_size);
    printf("\n");
    if (fd < 1)
    {
        int fd = open(path, O_WRONLY);

        if (fd < 1)
        {
            return -1;
        }
        write(fd, buf, buf_size);
    }
    else
    {
        read(fd, buf, buf_size);
    }

    close(fd);
    return 0;
}

char *run_bash_command(char *bash_command)
{
    FILE *fp;
    char path[1035];

    char *command = malloc((strlen(bash_command) + strlen(" 2>&1\0")) * sizeof(char));
    strcpy(command, "");
    strcat(command, bash_command);
    strcat(command, " 2>&1");
    /* Open the command for reading. */
    fp = popen(command, "r");
    free(command);

    if (fp == NULL)
    {
        return NULL;
    }

    char *output = malloc(sizeof(char));
    strcpy(output, "");
    /* Read the output a line at a time - output it. */
    while (fgets(path, sizeof(path), fp) != NULL)
    {
        output = realloc(output, (strlen(output) + strlen(path)) * sizeof(char));
        strcat(output, path);
    }
    /* close */
    pclose(fp);

    return output;
}