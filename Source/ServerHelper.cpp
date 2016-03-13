/*
        SOURCE FILE:        ServerHelper.cpp
        
        PROGRAM:            Linux Chat

        FUNCTIONS:

        PROGRAMMER:         Martin Minkov

        NOTES:              ServerHelper is responsible for providing the logic when there is
                            data received on a socket. It either accepts client connections
                            and adds them to the readable set or listens for data on established
                            sockets and provides the functionality of broadcasting to all clients.

*/

#include "ServerHelper.h"

/*
        FUNCTION:       bindAddressToSock       

        PROGRAMMER:     Martin Minkov

        INTERFACE:      void bindAddressToSock(struct sockaddr_in &server, int &port)
                            struct sockaddr_in &server:
                                - The structure that will be binded.
                            int &port:
                                - The port used to bind.

        RETURNS:        void

        NOTES:          Initializes the sockaddr_in struct to the specified port.

*/
void bindAddressToSock(struct sockaddr_in &server, int &port)
{
    bzero((char *)&server, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY); // Accept connections from any client 
}
/*
        FUNCTION:       handleConnect

        PROGRAMMER:     Martin Minkov

        INTERFACE:      void handleConnect(SelectHelper &helper, int &listen_sd)
                            SelectHelper &helper:
                                - The structure which holds all relevant set variables which
                                    is used for select()
                            int &listen_sd:
                                - The port that listens for new client connection requests

        RETURNS:        void

        NOTES:          Is called when the select() call detects data on the socket that listens
                        for new client connections. It will add them to the read set which is used
                        in select(), and also adds them to a client array which is later used to 
                        broadcast to all clients.
*/

void handleConnect(SelectHelper &helper, int &listen_sd)
{
	struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
	int new_sd;
    int i;

    if ((new_sd = accept(listen_sd, (struct sockaddr *) &client, &client_len)) == -1)
        perror("accept error");

    std::cout << "Remote Address: " << inet_ntoa(client.sin_addr) << " has connected." << std::endl;

    for (i = 0; i < FD_SETSIZE; i++)
    {
        //Available descriptor index found
        if (helper.client[i] < 0)
        {
            //Save new descriptor to client array
            helper.client[i] = new_sd;
            //Save new descriptor in connected array
            break;  
        }
    }
    //No availailble descriptor index found
    if (i == FD_SETSIZE)    
    {
        std::cout << "Too many clients currently" << std::endl; 
        return;
    }

    //Add new descriptor to set 
    FD_SET(new_sd, &helper.allset);

    if (new_sd > helper.maxfd)
       helper.maxfd = new_sd;

    if (i > helper.maxi) 
        helper.maxi = i;

    if (--helper.nready <= 0)
	    return;   // no more readable descriptors
}
/*
        FUNCTION:       handleData       

        PROGRAMMER:     Martin Minkov

        INTERFACE:      void handleData(SelectHelper &helper)
                            SelectHelper &helper:
                                - The structure which holds all relevant set variables which
                                     is used for select()


        RETURNS:        void

        NOTES:          Handles the logic for receiving data on the client ports. The function will
                        find the port that has data waiting, and then will read and broadcast to 
                        all other registered clients. 

*/
void handleData(SelectHelper &helper)
{
    int sockfd, bytes_to_read, n;
    char *bp, buf[BUFLEN];
    for (int i = 0; i <= helper.maxi; i++)
    {
		if ((sockfd = helper.client[i]) < 0)
        {
            continue;
        }

		if (FD_ISSET(sockfd, &helper.rset))
		{
			bp = buf;
			bytes_to_read = BUFLEN;
			if ((n = recv(sockfd, bp, bytes_to_read, 0)) > 0)
			{
				bytes_to_read -= n;
			}

            std::cout << "Received: " << bp << std::endl;

			//Broadcast to all clients
            for (int j = 0; j < LISTENQ - 1; j++)
            {
                if (helper.client[j] > 0)
                    send(helper.client[j], bp, BUFLEN, 0);   // echo to client
            }
			
			//Connection closed by client
			if (n == 0)
			{
				std::cout << "Closing socket :" << sockfd << std::endl;
				close(sockfd);
				FD_CLR(sockfd, &helper.allset);
				helper.client[i] = -1;
			}
		}
		// no more readable descriptors	
		//if (--helper.nready <= 0)
       	//	break;        	
	}	
}
