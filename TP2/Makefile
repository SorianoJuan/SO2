CC=gcc
CFLAGS=-Werror -Wall -pedantic

all: build/main

build/main: build/main.o
	$(CC) $(CFLAGS) build/main.o -lnetcdf -o build/main

build/main.o: src/main.c
	$(CC) $(CFLAGS) -c src/main.c -o build/main.o

clean:
	rm build/main*
