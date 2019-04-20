C++ = g++
FLAGS = -c -g -o
vpath src
vpath inc

all: main

main: main.o cache.o
	$(C++) -o main *.o

main.o: main.c
	$(C++) $(FLAGS) main.o main.c

cache.o: cache.c cache.h
	$(C++) $(FLAGS) cache.o cache.c

clean:
	rm -f *.o *.exe