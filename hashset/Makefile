CFLAGS = -Wall -pedantic -Os
LDFLAGS = -lm
SRC = hashset.c test_hashset.c
OUT = test_hashset

all: test_hashset hashset.o

test_hashset: hashset.o

test: test_hashset
	./test_hashset

clean:
	rm test_hashset hashset.o
