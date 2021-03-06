#ifndef CLIENTHELPER_H
#define CLIENTHELPER_H

#include <stdio.h>
#include <netdb.h>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <string.h>
#include <vector>

#define BUFLEN 1024

int setupAddrStruct(struct sockaddr_in &server, struct hostent *hp,
	char *hostname, int &port);

void sendMessage(int &connect_sd, char * message);

#endif // CLIENTHELPER_H

