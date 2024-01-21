SRC = arena.c test_arena.c
CFLAGS = -std=c99 -pedantic -Wall -Os

all: arena.o test_arena

arena.o: arena.c arena.h Makefile

test_arena: test_arena.c arena.o

test: test_arena
	./test_arena

clean:
	rm -f arena.o test_arena

