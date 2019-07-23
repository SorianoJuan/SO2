CC=gcc
CFLAGS=-Werror -Wall -pedantic

all: build/client build/server

build/client: build/client.o
	$(CC) $(CFLAGS) build/client.o -o build/client

build/client.o: src/client/main.c
	$(CC) $(CFLAGS) -c src/client/main.c -o build/client.o

build/server: build/server.o
	$(CC) $(CFLAGS) build/server.o -pthread -o build/server

build/server.o: src/server/main.c
	$(CC) $(CFLAGS) -c src/server/main.c -o build/server.o

clean:
	rm build/client*
	rm build/server*
