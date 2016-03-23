/*
        SOURCE FILE:        clienthelper.cpp
        
        PROGRAM:            Linux Chat

        FUNCTIONS:

        PROGRAMMER:         Alvin Man

        NOTES:              clienthelper ...

*/

#include "clienthelper.h"

int setupAddrStruct(struct sockaddr_in &server, struct hostent *hp,
	char *hostname, int &port)
{
	bzero((char *)&server, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    if ((hp = gethostbyname(hostname)) == NULL){
        fprintf(stderr, "Unknown server address\n");
        return -1;
    }
    bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);
    return 0;
}

void sendMessage(int &connect_sd, char * message){
	send (connect_sd, message, BUFLEN, 0);
}

//void receiveMessage(int &connect_sd){
//	int bytes_to_read;
//	int n;
//	char *bp;
//    char buf[BUFLEN];

//    fprintf(stderr, "inside receiveMessage");

//    bp = buf;
//    bytes_to_read = BUFLEN;
//    if ((n = recv(connect_sd, bp, bytes_to_read, 0)) > 0)
//    {
//        bytes_to_read -= n;
//    }
//    fprintf(stderr, buf);
//}
