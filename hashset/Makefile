# CFLAGS = -std=c99 -Wall -pedantic -O0 -g -D_DEFAULT_SOURCE
CFLAGS = -std=c99 -Wall -pedantic -O3 -D_DEFAULT_SOURCE
LDFLAGS = -lm
SRC = hashset.c test_hashset.c
OUT = test_hashset

all: test_hashset hashset.o

hashset.o: hashset.c hashset.h Makefile
	$(CC) $(CFLAGS) -c -o $@ $< ${LDFLAGS}

test_hashset: hashset.o

test: test_hashset
	./test_hashset

clean:
	rm test_hashset hashset.o
