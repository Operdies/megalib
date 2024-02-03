CFLAGS = -O0 -g -D_DEFAULT_SOURCE -std=c99 -Wall -Wextra -Werror -pedantic
CFLAGS = -O3 -D_DEFAULT_SOURCE -std=c99 -Wall -Wextra -Werror -pedantic
SRC = test_benchmark.c benchmark.h
OUT = test_benchmark

all: ${OUT}

test_benchmark: test_benchmark.c benchmark.h Makefile
	${CC} ${CFLAGS} -o $@ $< ${LDFLAGS}

test: all
	./test_benchmark
