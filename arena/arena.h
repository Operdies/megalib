#ifndef __ARENA_H
#define __ARENA_H

#ifndef ARENA_COMMIT
#define ARENA_COMMIT (1l<<36)
#endif

#include <stddef.h>

typedef struct {
	size_t size;      // total number of bytes that can be virtually addressed
	size_t cursor;    // pointer from the base of the arena pointer where new data is allocated
	size_t committed; // number of bytes actually committed. Increased as needed in arena_alloc
	char buffer[];    // the start of user allocated data
} arena;

// allocate a new arena
arena* mk_arena(void);
// Unmap the memory associated with an arena, including the arena itself
void destroy_arena(arena *a);
// allocate nmemb * sz bytes of memory
void *arena_alloc(arena *a, size_t nmemb, size_t size);

#ifdef ARENA_IMPLEMENTATION
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
	if (a == MAP_FAILED)
		return NULL;

	// Allocate a single page to store information about the arena
	if (mprotect(a, PAGESIZE, PROT_READ|PROT_WRITE))
		return NULL;

	*a = (arena) { .size = ARENA_COMMIT, .cursor = 0, .committed = PAGESIZE };
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
	void *ret = a->buffer + a->cursor;

	size = nmemb * mem_size;
	required = a->cursor + size + offsetof(arena, buffer);

	if (required > a->size) {
		return NULL;
	}

	if (required > a->committed) {
		// mprotect must be page aligned -- claim as many whole pages as needed
		to_commit = required - (required % __pagesize) + __pagesize;
		if (mprotect((char*)a + a->committed, to_commit, PROT_READ|PROT_WRITE))
			return NULL;
		a->committed += to_commit;
	}

	a->cursor += size;
	return ret;
}

#endif // ARENA_IMPLEMENTATION
#endif // __ARENA_H

