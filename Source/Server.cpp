#include "Server.h"

int main(int argc, char* argv[])
{
	int i, arg;
	int listen_sd, port;
	struct sockaddr_in server;
	SelectHelper set;
	
	//Check if port is specified in command line
	switch(argc)
	{
		case 1:
			port = SERVER_TCP_PORT; 	  //port 7000
			break;
		case 2:
			port = atoi(argv[1]);	      // Get user specified port	
	}
	// Create a stream socket
	if ((listen_sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		perror("Cannot Create Socket!");

	// set SO_REUSEADDR so port can be resused imemediately after exit, i.e., after CTRL-c
    arg = 1;
    if (setsockopt (listen_sd, SOL_SOCKET, SO_REUSEADDR, &arg, sizeof(arg)) == -1)
		perror("setsockopt");

    //Bind address to the socket
    bindAddressToSock(server, port); 

    if (bind(listen_sd, (struct sockaddr *)&server, sizeof(server)) == -1)
		perror("bind error");

    // Listen for connections
    // queue up to LISTENQ connect requests
    listen(listen_sd, LISTENQ);

    set.maxfd   = listen_sd;  	                 // initialize
    set.maxi    = -1;                        // index into client[] array

    for (i = 0; i < FD_SETSIZE; i++)
            set.client[i] = -1;              // -1 indicates available entry

    FD_ZERO(&set.allset);
    FD_SET(listen_sd, &set.allset);

    while (TRUE)
    {
        set.rset = set.allset;               // structure assignment
        set.nready = select(set.maxfd + 1, &set.rset, NULL, NULL, NULL);

        if (FD_ISSET(listen_sd, &set.rset)) // new client connection
        {
            //Start Update Host List - Descriptor Thread
			std::thread acceptThread(handleConnect, std::ref(set), std::ref(listen_sd));
			acceptThread.join();	
        }
        else
        {
            //Start Read Data Thread
            std::cout << "Creating data thread" << std::endl;
			std::thread dataThread(handleData, std::ref(set));
			dataThread.join();
        }
    } 
    return 1;
}
