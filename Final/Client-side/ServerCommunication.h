#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define SERVER_IP "172.17.2.28"
#define MAXLINE 1024
#define TCP_PORT 8080
#define UDP_PORT 0 //random port

int sendTCPMessage(char message[], char *IP, int port);
void sendUDPPacket(int *sockfd, struct sockaddr_in *cliaddr, socklen_t len, char *response);
char *receiveUDPPacket(int *sockfd, struct sockaddr_in *cliaddr, socklen_t *len);
int bindUDPSocket(int *sockfd, int port, struct sockaddr_in *cliaddr);
