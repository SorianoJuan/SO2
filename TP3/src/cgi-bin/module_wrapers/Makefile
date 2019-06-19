CFLAGS = -Wall -Werror -pedantic
CC=gcc

all: insmod rmmod clean

insmod: insmod.o
	sudo $(CC) -o insmod insmod.o
	sudo chmod a+s insmod

rmmod: rmmod.o
	sudo $(CC) -o rmmod rmmod.o
	sudo chmod a+s rmmod

rmmod: rmmod.c

insmod: insmod.c

clean:
	rm -f *.o *~
