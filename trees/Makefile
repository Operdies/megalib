CFLAGS = -D_DEFAULT_SOURCE -std=c99 -Wall -Wextra -Werror -pedantic -Wno-missing-field-initializers
SRC = test_trees.c bst.c
OUT = test_trees

all: test_trees bst

test_trees: test_trees.c tree.h Makefile
	${CC} ${CFLAGS} -o $@ $< ${LDFLAGS}

test: test_trees
	./$<

bst: bst.c Makefile
	${CC} ${CFLAGS} -o $@ $< ${LDFLAGS}

run_bst: bst
	./bst
