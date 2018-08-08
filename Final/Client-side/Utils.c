#include "Utils.h"

int string_split(char input[], char **argument, const char delimiter)
{
    int index, to_return = 0;
    char *position = strchr(input, delimiter);
    index = (position - input) / sizeof(char);

    if (position != NULL)
    {
        char *argument1 = position + sizeof(char);
        *argument = malloc(strlen(argument1) * sizeof(char));
        strcpy(*argument, argument1);
    }
    else
        to_return = -1;

    input[index] = '\0';

    return to_return;
}