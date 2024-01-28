#ifndef __HASHSET_H
#define __HASHSET_H

#ifndef RESIZE_THRESHOLD
#define RESIZE_THRESHOLD 0.75
#endif

#include <stddef.h>
#include <stdbool.h>
#include "../bitmap/bitmap.h"

typedef union {
  void    *pointer;
  size_t  integer;
  char    *string;
} hashset_key;

typedef hashset_key hashset_value;

typedef struct {
  hashset_key key;
  hashset_value value;
} kvp_t;

typedef size_t (*hashfunc_t)(const hashset_key);

typedef size_t (*cmpfunc_t)(const hashset_key, const hashset_key);

typedef char* (*formatfunc)(kvp_t kvp);

typedef struct {
  size_t count;
  size_t capacity;
  hashfunc_t hashfunc;
  cmpfunc_t cmpfunc;
  hashset_key *keys;
  hashset_value *values;
  bitmap_t *occupied;
} hashset;

// Returns true if hashset contains the specified key
bool hashset_contains(const hashset *h, const hashset_key key);
// Add a kvp_t to hashset if it doesn't already exist, returning true if a value was added.
bool hashset_add(hashset *h, const kvp_t);
// Set a kvp_t in hashset, returning true if a value was replaced. The replaced value is returned in *removed.
bool hashset_set(hashset *h, const kvp_t, hashset_value *removed);
// Remove a kvp_t from hashset, returning true if the value was removed. The removed value is returned in *removed.
bool hashset_remove(hashset *h, const hashset_key key, hashset_value *removed);
// Make a new hashset with a given hash function.
void mk_hashset(hashset *h, const hashfunc_t hashfunc, const cmpfunc_t cmpfunc, size_t initial_size);
// Destroy a hashset, freeign keys and values
void destroy_hashset(hashset *h);
// Hash function suitable for integer keys
size_t hash_integer(const hashset_key);
// Hash function suitable for string keys
size_t hash_string(const hashset_key);
// Hash function suitable for pointer keys
size_t hash_pointer(const hashset_key);
// Print each key in the hashset, formatting the kvp with the provided formatter
void hashset_print(hashset *h, formatfunc f);
// Wrapper around strcmp from <string.h> that accepts null pointers
size_t hashmap_strcmp(const hashset_key a, const hashset_key b);

#ifdef HASHSET_IMPLEMENTATION

#define BITMAP_IMPLEMENTATION
#include "../bitmap/bitmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define LENGTH(X) (sizeof(X) / sizeof(X[0]))

size_t primes[] = { 53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317, 196613, 393241, 786433, 1572869, 3145739, 6291469, 12582917, 25165843, 50331653, 100663319, 201326611, 402653189, 805306457, 1610612741 };

size_t hash_integer(hashset_key key) {
  return key.integer;
}

size_t hash_pointer(hashset_key key) {
  return (size_t)key.pointer / sizeof(size_t);
}

size_t hash_string(hashset_key key) {
  char *str = key.string;
  size_t hash, idx;
  hash = 0;
  for (idx = 0; *str; idx++, str++) {
    size_t prime = primes[idx % LENGTH(primes)];
    hash = (hash ^ prime) + (*str * prime);
  }
  return hash;
}

size_t hashmap_strcmp(const hashset_key a, const hashset_key b) {
  if (a.string == NULL || b.string == NULL)
    return a.integer - b.integer;
  return strcmp(a.string, b.string);
}

// Assume p > 3 and p is odd
bool is_prime(size_t p) {
  size_t limit = (size_t)sqrt(p);
  for (size_t c = 3; c < limit; c += 2) {
    if (p % c == 0)
      return 0;
  }
  return 1;
}

// Calculate the next prime that is at least as large as p * 2
size_t next_prime(size_t p) {
  size_t next;
  for (next = p * 2 + 1; !is_prime(next); next += 2);
  return next;
}

size_t default_equality(hashset_key a, hashset_key b) {
  return a.integer - b.integer;
}

void mk_hashset(hashset *h, hashfunc_t hashfunc, cmpfunc_t cmpfunc, size_t sz) {
  *h = (hashset) { 0 };
  h->capacity = sz;
  h->hashfunc = hashfunc;
  h->cmpfunc = cmpfunc ? cmpfunc : default_equality;
  h->keys = calloc(sz, sizeof(hashset_key));
  h->values = calloc(sz, sizeof(hashset_value));
  h->occupied = mk_bitmap(sz);
}

void destroy_hashset(hashset *h) {
  free(h->keys);
  free(h->values);
  destroy_bitmap(h->occupied);
}

bool hashset_contains_slot(const hashset *h, const hashset_key key, size_t *index) {
  size_t hash, slot;
  hash = h->hashfunc(key);
  slot = hash % h->capacity;
  while (bit_set(h->occupied, slot)) {
    if (h->cmpfunc(h->keys[slot], key) == 0) {
      if (index) 
        *index = slot;
      return true;
    }
    slot = (slot + 1) % h->capacity;
  }
  return false;
}

bool hashset_contains(const hashset *h, const hashset_key key) {
  return hashset_contains_slot(h, key, NULL);
}

bool hash_insert(hashset *h, kvp_t kvp) {
  size_t hash, slot;
  hash = h->hashfunc(kvp.key);
  slot = hash % h->capacity;
  while (bit_set(h->occupied, slot)) {
    if (h->cmpfunc(h->keys[slot], kvp.key) == 0) {
      return false;
    }
    slot = (slot + 1) % h->capacity;
  }
  set_bit(h->occupied, slot, 1);
  h->keys[slot] = kvp.key;
  h->values[slot] = kvp.value;
  h->count++;
  return true;
}

void enlarge(hashset *h) {
  size_t next_size = next_prime(h->capacity);
  hashset newset;
  mk_hashset(&newset, h->hashfunc, h->cmpfunc, next_size);
  for (size_t i = 0; i < h->capacity; i++) {
    if (bit_set(h->occupied, i)) {
      hash_insert(&newset, (kvp_t) { .key = h->keys[i], .value = h->values[i] });
    }
  }
  destroy_hashset(h);
  *h = newset;
}

// // Add a kvp_t to hashset if it doesn't already exist, returning true if a value was added.
bool hashset_add(hashset *h, const kvp_t kvp) {
  float fullness = h->capacity ? (float)h->count / h->capacity : 1;

  // Enlarge keys / values if we exceed the specified threshold
  if (fullness > RESIZE_THRESHOLD) {
    enlarge(h);
  }
  return hash_insert(h, kvp);
}

bool hashset_remove(hashset *h, const hashset_key key, hashset_value *removed) {
  size_t index;
  if (hashset_contains_slot(h, key, &index)) {
    *removed = h->values[index];
    h->values[index] = (hashset_value) { 0 };
    h->keys[index] = (hashset_key) { 0 };
    set_bit(h->occupied, index, 0);
    h->count--;
    return true;
  }
  return false;
}

bool hashset_set(hashset *h, const kvp_t kvp, hashset_value *removed) {
  bool value_removed = hashset_remove(h, kvp.key, removed);
  hashset_add(h, kvp);
  return value_removed;
}

void hashset_print(hashset *h, formatfunc f) {
  for (size_t i = 0; i < h->capacity; i++) {
    if (bit_set(h->occupied, i)) {
      printf("%3zu: %s\n", i, f((kvp_t) { .key = h->keys[i], .value = h->values[i] }));
    }
  }
}

#endif // HASHSET_IMPLEMENTATION
#endif // __HASHSET_H
