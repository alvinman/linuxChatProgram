CC = g++ -std=c++11 -Wall -ggdb -pthread

server: Server.o ServerHelper.o 
	$(CC) -o Server Server.o ServerHelper.o 

Server.o: Server.cpp Server.h 
	$(CC) -c Server.cpp

ServerHelper.o: ServerHelper.cpp ServerHelper.h
	$(CC) -c ServerHelper.cpp

clean: 
	rm -f *.o core.* Server
