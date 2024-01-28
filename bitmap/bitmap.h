#ifndef __BITMAP_H
#define __BITMAP_H

#include <stddef.h>
#include <stdbool.h>

typedef unsigned char bitmap_t;
typedef unsigned char bit_t;

// Make a new bitmap with the given capacity
bitmap_t *mk_bitmap(size_t capacity);
// Destroy the given bitmap
void destroy_bitmap(bitmap_t *bitmap);
// Return true if the given bit is set in the bitmap.
bool bit_set(const bitmap_t *bitmap, size_t idx);
// Turn the given bit on or off
void set_bit(bitmap_t *bitmap, size_t idx, bit_t value);
// Toggle the given bit
void toggle_bit(bitmap_t *bitmap, size_t idx);

#endif // __BITMAP_H

#ifdef BITMAP_IMPLEMENTATION
#undef BITMAP_IMPLEMENTATION

#include <stdlib.h>
#include <stdio.h>

bitmap_t *mk_bitmap(size_t capacity) {
  return calloc(capacity / (8 * sizeof(bitmap_t)) + 1, sizeof(bitmap_t));
}

void destroy_bitmap(bitmap_t *bitmap) {
  free(bitmap);
}

size_t bitmap_sum(bitmap_t *bitmap, size_t bitmap_size) {
  size_t sum = 0;
  for (size_t i = 0; i < bitmap_size; i++)
    sum += bit_set(bitmap, i) ? 1 : 0;
  return sum;
}

inline bool bit_set(const bitmap_t *bitmap, size_t idx) {
  return (bitmap[idx/8] >> (idx % 8)) & 1;
}

void set_bit(bitmap_t *bitmap, size_t idx, bit_t value) {
  size_t index, bit;
  index = idx / (sizeof(bitmap_t) * 8);
  bit = idx % (sizeof(bitmap_t) * 8);
  if (value) {
    bitmap[index] |= (1l << bit);
  } else {
    bitmap[index] &= ~0l ^ (1l << bit);
  }
}

void toggle_bit(bitmap_t *bitmap, size_t idx) {
  size_t index, bit;
  index = idx / (sizeof(bitmap_t) * 8);
  bit = idx % (sizeof(bitmap_t) * 8);
  bitmap[index] ^= 1l << bit;
}

void print_bitmap(bitmap_t *bitmap, size_t size) {
  puts("    0 1 2 3 4 5 6 7");
  for (size_t i = 0; i < size; i+=8) {
    printf("%2zu:", i / 8);
    for (size_t j = 0; j < 8 && (i+j) < size; j++) {
      printf(" %d", bit_set(bitmap, i + j));
    }
    puts("");
  }
}

#endif // BITMAP_IMPLEMENTATION
