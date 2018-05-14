baash: main.o evalfunc.o archfunc.o
	gcc main.o evalfunc.o archfunc.o -o baash

main.o: main.c
	gcc -c main.c

evalfunc.o: evalfunc.c evalfunc.h
	gcc -c evalfunc.c

archfunc.o: archfunc.c archfunc.h
	gcc -c archfunc.c

clean:
	rm *.o baash
