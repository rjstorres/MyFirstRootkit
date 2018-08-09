#include "ModuleCommunication.h"

int main(int argc, char *argv[])
{
    char *input;
    input= malloc(strlen(argv[1])*sizeof(char));
    strncpy ( input, argv[1], sizeof(input) );
    start_command(input);
    return 0;
}

