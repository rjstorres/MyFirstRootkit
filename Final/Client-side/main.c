#include "ServerCommunication.h"

#define BASH "bash_cmd"
#define MODULE "module_cmd"

int main(int argc, char *argv[])
{
    //start_command(argv[1]);
    //free(run_bash_command(argv[1]));
   /* char *type=argv[1];
    char *cmd;
    string_split(type,&cmd,':');
    printf("\n%s - %s\n", type, cmd);
    if(strcmp(MODULE,type)==0)
        start_module_command(cmd);
    else if (strcmp(BASH,type)==0)
        free(start_bash_command(cmd));
    else printf("Error\n");
    */

    /*char message[]= "I'M ALIVE";
    sendTCPMessage(message, SERVER_IP, TCP_PORT);*/

    /*int sockfd;
    struct sockaddr_in cliaddr;
    bindUDPSocket(&sockfd,9999,&cliaddr);
    int len;
    char *buffer= receiveUDPPacket(&sockfd,&cliaddr, &len);
    free(buffer);
    sendUDPPacket(&sockfd,&cliaddr,&len, response);*/


    
    return 0;
}
