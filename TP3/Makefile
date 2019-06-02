CC=gcc
CFLAGS=-Werror -Wall -pedantic -fopenmp

all: build/procedural build/openmp
	rm build/*.o

build/procedural: build/procedural.o
	$(CC) $(CFLAGS) build/procedural.o -lm -o build/procedural

build/procedural.o: src/procedural/main.c
	$(CC) $(CFLAGS) -c src/procedural/main.c -o build/procedural.o

build/openmp: build/openmp.o
	$(CC) $(CFLAGS) build/openmp.o -lm -o build/openmp

build/openmp.o: src/openmp/main.c
	$(CC) $(CFLAGS) -c src/openmp/main.c -o build/openmp.o

clean:
	rm build/procedural
	rm build/openmp