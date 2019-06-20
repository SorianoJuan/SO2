CC=gcc
CFLAGS=-Werror -Wall -pedantic

all: modulos systeminfo goes

modulos: modulos.o
	$(CC) $(CFLAGS) modulos.o -o modulos.cgi

modulos.o: modulos.c
	$(CC) $(CFLAGS) -c modulos.c -o modulos.o

systeminfo: systeminfo.o
	$(CC) $(CFLAGS) systeminfo.o -o systeminfo.cgi

systeminfo.o: systeminfo.c
	$(CC) $(CFLAGS) -c systeminfo.c -o systeminfo.o

goes: goes.o
	$(CC) $(CFLAGS) goes.o -o goes.cgi

goes.o: goes.c
	$(CC) $(CFLAGS) -c goes.c -o goes.o

clean:
	rm *.o modulos systeminfo goes 
