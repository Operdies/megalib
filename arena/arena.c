#include "arena.h"
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

static long __pagesize = 0;
#define PAGESIZE (__pagesize ? __pagesize : (__pagesize = sysconf(_SC_PAGESIZE)))


arena* mk_arena(void) {
	arena *a;

	// acquire virtual addressing for ARENA_COMMIT bytes (possibly more than the system has available)
	// The upper limit is system dependent.
	a = mmap(0, ARENA_COMMIT, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	if (!a)
		return NULL;

	// Allocate a single page to store information about the arena
	if (mprotect(a, PAGESIZE, PROT_READ|PROT_WRITE))
		return NULL;

	*a = (arena) { .size = ARENA_COMMIT, .cursor = offsetof(arena, buffer), .committed = PAGESIZE };
	return a;
}

void destroy_arena(arena *a) {
	if (munmap(a, a->size)) {
		perror("munmap:");
		exit(1);
	}
}

void* arena_alloc(arena *a, size_t nmemb, size_t mem_size) {
	size_t size, required, to_commit;

	size = nmemb * mem_size;
	required = a->cursor + size;

	if (required > a->size)
		return NULL;

	if (required > a->committed) {
		// mprotect must be page aligned -- claim as many whole pages as needed
		to_commit = required - (required % __pagesize) + __pagesize;
		if (mprotect((char*)a + a->committed, to_commit, PROT_READ|PROT_WRITE))
			return NULL;
		a->committed += to_commit;
	}

	a->cursor += size;
	return (char*)a + a->cursor - size;
}

