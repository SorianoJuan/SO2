CC=gcc
CFLAGS=-Werror -Wall -pedantic

all: build/client build/server build/baash

build/client: build/client.o
	$(CC) $(CFLAGS) build/client.o -pthread -o build/client

build/client.o: src/client/main.c
	$(CC) $(CFLAGS) -c src/client/main.c -o build/client.o

build/server: build/server.o
	$(CC) $(CFLAGS) build/server.o -o build/server

build/server.o: src/server/main.c
	$(CC) $(CFLAGS) -c src/server/main.c -o build/server.o

build/baash: src/baash/main.o src/baash/evalfunc.o src/baash/archfunc.o
	gcc src/baash/main.o src/baash/evalfunc.o src/baash/archfunc.o -o build/baash
	rm build/*.o

main.o: src/baash/main.c
	gcc -c src/baash/main.c

evalfunc.o: src/baash/evalfunc.c
	gcc -c src/baash/evalfunc.c

archfunc.o: src/baash/archfunc.c
	gcc -c src/baash/archfunc.c

clean:
	rm build/client*
	rm build/server*
	rm build/baash
	rm src/baash/*.o
