CC = g++
CFLAGS = -std=c++11

all: clientMain.c serverMain.c server.o client.o
	$(CC) $(CFLAGS) serverMain.c server.o -o game_server
	$(CC) $(CFLAGS) clientMain.c client.o -o game_client

server.o: server.c server.h common.h
	$(CC) $(CFLAGS) -c server.c

client.o: client.c client.h common.h
	$(CC) $(CFLAGS) -c client.c

clean:
	rm -f *.o game_server game_client