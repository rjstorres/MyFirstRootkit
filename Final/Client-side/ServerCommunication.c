#include "ServerCommunication.h"

int sendTCPMessage(char message[], char *IP, int port)
{
    int sock;
    struct sockaddr_in server;

    //Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }

    server.sin_addr.s_addr = inet_addr(IP);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    //Connect to remote server
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return -1;
    }

    if (send(sock, message, strlen(message), 0) < 0)
    {
        perror("Send failed");
        return -1;
    }

    close(sock);
    return 0;
}

int bindUDPSocket(int *sockfd, int port, struct sockaddr_in *cliaddr)
{
    struct sockaddr_in servaddr;

    // Creating socket file descriptor
    if ((*sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        return -1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(cliaddr, 0, sizeof(*cliaddr));

    // Filling server information
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);

    // Bind the socket with the server address
    if (bind(*sockfd, (const struct sockaddr *)&servaddr,
             sizeof(servaddr)) < 0)
    {
        perror("bind failed");
        return -1;
    }

    socklen_t slen = sizeof(servaddr);
    getsockname(*sockfd, (struct sockaddr *)&servaddr, &slen);
    return ntohs(servaddr.sin_port);
}

char *receiveUDPPacket(int *sockfd, struct sockaddr_in *cliaddr, socklen_t *len)
{
    char *buffer = malloc(MAXLINE);
    int n;
    n = recvfrom(*sockfd, (char *)buffer, MAXLINE,
                 MSG_WAITALL, (struct sockaddr *)cliaddr,
                 len);
    buffer[n] = '\0';

    char buffff[20];

    inet_ntop(AF_INET, &(cliaddr->sin_addr), buffff, *len);
    printf("%s - %d\n", buffff, ntohs(cliaddr->sin_port));
    return buffer;
}

void sendUDPPacket(int *sockfd, struct sockaddr_in *cliaddr, socklen_t len, char *response)
{
    sendto(*sockfd, (const char *)response, strlen(response),
           MSG_CONFIRM, (const struct sockaddr *)cliaddr,
           len);
}