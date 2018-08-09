#include "ModuleCommunication.h"

int main(int argc, char *argv[])
{
    char *input;
    strncpy ( input, argv[1], strlen(argv[1]) );
    start_command(input);
    return 0;
}

