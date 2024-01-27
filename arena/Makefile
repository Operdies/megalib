SRC = arena.c test_arena.c
CFLAGS = -std=c99 -pedantic -Wall -O0 -g -DARENA_IMPLEMENTATION

all: test_arena

test_arena: test_arena.c arena.h

test: test_arena
	./test_arena

clean:
	rm -f arena.o test_arena

