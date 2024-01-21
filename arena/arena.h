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

#ifdef ARENA_IMPLEMENTAITON
#include "arena.c"
#endif

#endif

