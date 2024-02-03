SRC = test_unittest.c
OUT = test_unittest

all: test_unittest

test_unittest: test_unittest.c unittest.h Makefile
	${CC} ${CFLAGS} -o $@ $< ${LDFLAGS}

test: test_unittest
	./$<
