#ifndef __HASHSET_H
#define __HASHSET_H

#ifndef RESIZE_THRESHOLD
#define RESIZE_THRESHOLD 0.75
#endif

#include <stddef.h>
#include <stdbool.h>

typedef unsigned long long i64;

typedef union {
  void     *pointer;
  i64      integer;
  double   real;
  char     *string;
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
  unsigned char *used;
} hashset;

// Returns true if hashset contains the specified key
bool hashset_get(const hashset *h, const hashset_key key, hashset_value *value);
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
size_t hashset_strcmp(const hashset_key a, const hashset_key b);

#ifdef HASHSET_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

const size_t coefficients[] = { 1073741827, 1073741831, 1073741833, 1073741839, 1073741843, 1073741857, 1073741891, 1073741909, 1073741939, 1073741953, 1073741969, 1073741971, 1073741987, 1073741993, 1073742037, 1073742053, 1073742073, 1073742077, 1073742091, 1073742113, 1073742169, 1073742203, 1073742209, 1073742223, 1073742233, 1073742259, 1073742277, 1073742289, 1073742343, 1073742353, 1073742361, 1073742391, 1073742403, 1073742463, 1073742493, 1073742517, 1073742583, 1073742623, 1073742653, 1073742667, 1073742671, 1073742673, 1073742707, 1073742713, 1073742721, 1073742731, 1073742767, 1073742773, 1073742811, 1073742851, 1073742853, 1073742881, 1073742889, 1073742913, 1073742931, 1073742937, 1073742959, 1073742983, 1073743007, 1073743037, 1073743049, 1073743051, 1073743079, 1073743091, 1073743093, 1073743123, 1073743129, 1073743141, 1073743159, 1073743163, 1073743189, 1073743199, 1073743207, 1073743243, 1073743291, 1073743303, 1073743313, 1073743327, 1073743331, 1073743337, 1073743381, 1073743387, 1073743393, 1073743397, 1073743403, 1073743417, 1073743421, 1073743427, 1073743457, 1073743459, 1073743469, 1073743501, 1073743507, 1073743513, 1073743543, 1073743577, 1073743591, 1073743633, 1073743739, 1073743757, };
const size_t n_coefficients = sizeof(coefficients) / sizeof(coefficients[0]);

size_t hash_integer(hashset_key key) {
  return key.integer * 2;
}

size_t hash_pointer(hashset_key key) {
  return key.integer / 4;
}

size_t hash_string(hashset_key key) {
  char *str = key.string;
  size_t hash, idx;
  hash = 0;
  for (idx = 0; *str; idx++, str++) {
    size_t prime = coefficients[idx % n_coefficients];
    hash ^= (*str * prime);
  }
  return hash;
}

size_t hashset_strcmp(const hashset_key a, const hashset_key b) {
  if (a.string == NULL || b.string == NULL)
    return a.integer - b.integer;
  return strcmp(a.string, b.string);
}

// Assume p > 3 and p is odd
bool is_prime(size_t p) {
  size_t limit = (size_t)sqrt(p);
  for (size_t c = 3; c <= limit; c += 2) {
    if (p % c == 0)
      return 0;
  }
  return 1;
}

// Calculate the next prime that is at least as large as p * 2
size_t next_size(size_t p) {
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
  if (sz) {
    h->keys = calloc(sz, sizeof(hashset_key));
    h->values = calloc(sz, sizeof(hashset_value));
    h->used = calloc(sz, 1);
  }
}

void destroy_hashset(hashset *h) {
  free(h->keys);
  free(h->values);
  free(h->used);
}

#define next_slot(slot) (((slot) + 1) % h->capacity)

bool hashset_contains_key(const hashset *h, const hashset_key key, size_t *index) {
  if (h->count == 0) return false;
  size_t hash, slot;
  hash = h->hashfunc(key);
  slot = hash % h->capacity;
  while (h->used[slot]) {
    if (h->cmpfunc(h->keys[slot], key) == 0) {
      if (index) 
        *index = slot;
      return true;
    }
    slot = next_slot(slot);
  }
  return false;
}

bool hashset_get(const hashset *h, const hashset_key key, hashset_value *value) {
  if (h->count == 0) return false;
  size_t index;
  if (hashset_contains_key(h, key, &index)) {
    *value = h->values[index];
    return true;
  }
  return false;
}

bool hash_insert(hashset *h, kvp_t kvp) {
  size_t hash, slot;
  hash = h->hashfunc(kvp.key);
  slot = hash % h->capacity;
  while (h->used[slot]) {
    if (h->cmpfunc(h->keys[slot], kvp.key) == 0) {
      return false;
    }
    slot = next_slot(slot);
  }
  h->used[slot] = 1;
  h->keys[slot] = kvp.key;
  h->values[slot] = kvp.value;
  h->count++;
  return true;
}

void enlarge(hashset *h) {
  size_t new_size = next_size(h->capacity);
  hashset newset = *h;
  mk_hashset(&newset, h->hashfunc, h->cmpfunc, new_size);
  for (size_t i = 0; i < h->capacity; i++) {
    if (h->used[i]) {
      hash_insert(&newset, (kvp_t) { .key = h->keys[i], .value = h->values[i] });
    }
  }
  destroy_hashset(h);
  *h = newset;
}

bool hashset_add(hashset *h, const kvp_t kvp) {
  float fullness = h->capacity ? (float)h->count / h->capacity : 1;

  // Enlarge keys / values if we exceed the specified threshold
  if (fullness > RESIZE_THRESHOLD || h->count == h->capacity) {
    enlarge(h);
  }
  return hash_insert(h, kvp);
}

bool hashset_remove(hashset *h, const hashset_key key, hashset_value *removed) {
  size_t slot;
  if (!hashset_contains_key(h, key, &slot))
    return false;
  if (removed)
    *removed = h->values[slot];

  h->used[slot] = 0;
  h->count--;

  slot = next_slot(slot);
  while (h->used[slot]) {
    h->used[slot] = 0;
    h->count--;
    kvp_t kvp = { .key = h->keys[slot], .value = h->values[slot] };
    hash_insert(h, kvp);
    slot = next_slot(slot);
  }

  return true;
}

bool hashset_set(hashset *h, const kvp_t kvp, hashset_value *removed) {
  size_t index;
  if (hashset_contains_key(h, kvp.key, &index)) {
    if (removed)
      *removed = h->values[index];
    h->keys[index] = kvp.key;
    h->values[index] = kvp.value;
    return true;
  } else {
    return hashset_add(h, kvp);
  }
}

void hashset_print(hashset *h, formatfunc f) {
  for (size_t i = 0; i < h->capacity; i++) {
    if (h->used[i]) {
      printf("%3zu: %s\n", i, f((kvp_t) { .key = h->keys[i], .value = h->values[i] }));
    }
  }
}

#endif // HASHSET_IMPLEMENTATION
#endif // __HASHSET_H
