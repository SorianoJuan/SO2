CC=gcc
CFLAGS=-Werror -Wall -pedantic

all: build/client

build/client: build/client.o
	$(CC) $(CFLAGS) build/client.o -pthread -o build/client

build/client.o: src/client/main.c
	$(CC) $(CFLAGS) -c src/client/main.c -o build/client.o

clean:
	rm build/client*
