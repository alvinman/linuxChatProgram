/*
        SOURCE FILE:        clienthelper.cpp
        
        PROGRAM:            Linux Chat

        FUNCTIONS:          int setupAddrStruct(struct sockaddr_in &server, struct hostent *hp,
                                char *hostname, int &port);
                            void sendMessage(int &connect_sd, char * message);

        PROGRAMMER:         Alvin Man

        DESIGNER:           Alvin Man

        NOTES:              Contains some helper functions used by client.
*/
#include "clienthelper.h"

/*
        FUNCTION:           setupAddrStruct

        PROGRAMMER:         Alvin Man

        DESIGNER:           Alvin Man

        INTERFACE:          int setupAddrStruct(struct sockaddr_in &server, struct hostent *hp,
                                char *hostname, int &port);
                                &server - address struct of server
                                *hp - host information struct
                                *hostname - the hostname string
                                &port - port number

        RETURNS:            int

        NOTES:              Sets up the address struct based on the hostname and port number
*/
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

/*
        FUNCTION:           sendMessage

        PROGRAMMER:         Alvin Man

        DESIGNER:           Alvin Man

        INTERFACE:          void sendMessage(int &connect_sd, char * message);
                                &connect_sd - handle to the socket
                                message - message to send to server

        RETURNS:            void

        NOTES:              Uses the send() call to send a message to the socket.
*/
void sendMessage(int &connect_sd, char * message){
	send (connect_sd, message, BUFLEN, 0);
}
