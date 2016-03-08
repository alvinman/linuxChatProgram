#include "ServerHelper.h"

void bindAddressToSock(struct sockaddr_in &server, int &port)
{
    bzero((char *)&server, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY); // Accept connections from any client 
}
void handleConnect(SelectHelper helper, int listen_sd, int new_sd)
{
	struct sockaddr_in client;
    socklen_t client_len = sizeof(client);

    if ((new_sd = accept(listen_sd, (struct sockaddr *) &client, &client_len)) == -1)
        perror("accept error");

    std::cout << "Remote Address: " << inet_ntoa(client.sin_addr) << std::endl;
    for (int i = 0; i < FD_SETSIZE; i++)
    {
        //Available descriptor index found
        if (helper.client[i] < 0)
        {
            //Save new descriptor to client array
            helper.client[i] = new_sd;
            break;  
        }
        //No availailble descriptor index found
        if (i == FD_SETSIZE)    
        {
            std::cout << "Too many clients currently" << std::endl; 
            break;
        }

        //Add new descriptor to set 
        FD_SET(new_sd, &helper.allset);

        if (new_sd > helper.maxfd)
           //For Select
           helper.maxfd = new_sd;
    
        if (i > helper.maxi) 
            //New max index in client array
            helper.maxi = i;

         if (--helper.nready <= 0)
            continue;   // no more readable descriptors
    }
}
void handleData(SelectHelper helper)
{
    int sockfd, bytes_to_read, n;
    char *bp, buf[BUFLEN];
    for (int i = 0; i <= helper.maxi; i++)
    {
        if ((sockfd = helper.client[i]) < 0)
            continue;
		if (FD_ISSET(sockfd, &helper.rset))
		{
			bp = buf;
			bytes_to_read = BUFLEN;
			while ((n = read(sockfd, bp, bytes_to_read)) > 0)
			{
				bp += n;
				bytes_to_read -= n;
			}
			//Broadcast to all clients
			
			//Connection closed by client
			if (n == 0)
			{
				close(sockfd);
				FD_CLR(sockfd, &helper.allset);
				helper.client[i] = -1;
			}
		}
		// no more readable descriptors	
		if (--helper.nready <= 0)
       		break;        	
	}	
}
