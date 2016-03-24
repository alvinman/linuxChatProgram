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

	//Add new client hostname to list
	helper.connectedClients.insert(std::pair<int, std::string>(new_sd, inet_ntoa(client.sin_addr)));
	
	printClientList(std::ref(helper));

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
    int sockfd, bytes_to_read, n, serverRequest = 0; 
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
			memset(bp, 0, BUFLEN);
			bytes_to_read = BUFLEN;
			if ((n = recv(sockfd, bp, bytes_to_read, 0)) > 0)
			{
				bytes_to_read -= n;
			}
            std::cout << "Received: " << bp << std::endl;

			//Check if Username message
			if ((serverRequest = checkServerRequest(sockfd, bp)) == 1)
			{
				std::string clientTable = constructClientTable();
				//Broadcast chat message to all other clients
				for (int j = 0; j < LISTENQ - 1; j++)
				{
					//Send client table to all clients
					if (helper.client[j] > 0)
						send(helper.client[j], clientTable.c_str(), BUFLEN, 0);   // echo to client
				}
			}
			else if (serverRequest == 2)
			{
				//Broadcast chat message to all other clients
				for (int j = 0; j < LISTENQ - 1; j++)
				{
					if (sockfd != helper.client[j] && helper.client[j] > 0)
						send(helper.client[j], bp, BUFLEN, 0);   // echo to client
				}
			}

			//Connection closed by client
			if (n == 0)
			{	
				//Check connected host names list for socket and remove entry
				if (helper.connectedClients.find(sockfd) != helper.connectedClients.end())
				{
					helper.connectedClients.erase(sockfd);
					printClientList(std::ref(helper));
				}
				//Check user name list for socket and remove entry
				if (clientUsernames.find(sockfd) != clientUsernames.end())
				{
					clientUsernames.erase(sockfd);
					std::string clientTable = constructClientTable();	
					for (int j = 0; j < LISTENQ - 1; j++)
					{
						//Send client table to all clients
						if (helper.client[j] > 0)
							send(helper.client[j], clientTable.c_str(), BUFLEN, 0);   // echo to client
					}

				}
				std::cout << "Closing socket :" << sockfd << std::endl;
				close(sockfd);
				FD_CLR(sockfd, &helper.allset);
				helper.client[i] = -1;
			}
		}
	}	
}
/*
        FUNCTION:      	printClientList 

        PROGRAMMER:     Martin Minkov

        INTERFACE:      void printClientList(SelectHelper &helper)
							SelectHelper &helper:
                                - The structure which holds all relevant lists which
                                    contains all connected clients 

        RETURNS:        void

        NOTES:         	Prints all connected clients to stdout 

*/

void printClientList(SelectHelper &helper)
{
	//Print updated list to stdout
	for (auto it = helper.connectedClients.cbegin(); it != helper.connectedClients.cend(); it++)
	{
		std::cout << "Host Name: " << it->second << std::endl;	
	}
}
/*
        FUNCTION:      	checkServerRequest 

        PROGRAMMER:     Martin Minkov

        INTERFACE:      int checkServerRequest(int port, char* bp)
                            SelectHelper &helper:
                                - int port:
									- Used to store receiving port
								- char* bp: 
									- The buffer that holds the username


        RETURNS:       int: Returns 1 if username is found, otherwise 0 

        NOTES:         Parses message to check if its username client message. If USERNAME: is detected
						the user name will be parsed and stored inside a list.

*/

int checkServerRequest(int port, char* bp)
{
	std::string tempBuf(bp);
	std::size_t found = tempBuf.find("USERNAME: ");

	//User joined message
	if (found != std::string::npos)
	{
		//Add new client to list
		tempBuf = tempBuf.substr(found+10);
		clientUsernames.insert(std::pair<int, std::string>(port, tempBuf));
		return 1;
	}
	found = tempBuf.find("MESSAGE: ");
	if (found != std::string::npos)
		return 2;

	return 0;
}
/*
        FUNCTION:       constructClientTable

        PROGRAMMER:     Martin Minkov

        INTERFACE:      std::string constructClientTable()

        RETURNS:        void

        NOTES:          Contucts a user list out of all currently connected clients.
*/

std::string constructClientTable()
{
	std::string temp = "";
	int i = 1;	

	for (auto it = clientUsernames.cbegin(); it != clientUsernames.cend(); it++, i++)
	{
		temp += "USER: " +  it->second + " "; 
	}
	return temp;
}
