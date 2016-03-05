CC = g++ -std=c++11 -Wall -ggdb -pthread

server: Server.o
	$(CC) -o Server Server.o

Server.o: Server.cpp Server.h
	$(CC) -c Server.cpp

clean: 
	rm -f *.o core.* Server
