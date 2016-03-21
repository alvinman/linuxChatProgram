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

#define BUFLEN 80

void setupAddrStruct(struct sockaddr_in &server, struct hostent *hp,
	char *hostname, int &port);

void sendMessage(int &connect_sd, char * message);
//void receiveMessage(int &connect_sd);

#endif // CLIENTHELPER_H

