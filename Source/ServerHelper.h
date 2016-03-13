#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <string.h>

#define SERVER_TCP_PORT 7000        // Default port
#define BUFLEN  80                  //Buffer length
#define TRUE    1
#define LISTENQ 10
#define MAXLINE 4096

typedef struct SelectHelper
{
    int maxi, maxfd, nready;
    fd_set rset, allset;
    int client[FD_SETSIZE];
} SelectHelper;

void bindAddressToSock(struct sockaddr_in &server, int &port);

void handleConnect(SelectHelper &helper, int &listen_sd);

void handleData(SelectHelper &helper);
