#define BITMAP_IMPLEMENTATION
#include "bitmap.h"

#define LENGTH(X) (sizeof(X) / sizeof(X[0]))
#define ASSERT(X) if (!(X)) { printf("%s:%d: Assertion failed: %s\n", __FILE__, __LINE__, #X); exit(1); }
#define ASSERT_EQUAL(X, Y) if ((X) != (Y)) { printf("%s:%d: Assertion failed: %s != %s (%zu != %zu)\n", __FILE__, __LINE__, #X, #Y, (size_t)(X), (size_t)(Y)); exit(1); }

size_t bitmap_sum(bitmap_t *bitmap, size_t bitmap_size) {
  size_t sum = 0;
  for (size_t i = 0; i < bitmap_size; i++)
    sum += bit_set(bitmap, i) ? 1 : 0;
  return sum;
}

int main(void) {
  size_t bitmap_size = 20;
  bitmap_t *bitmap = mk_bitmap(bitmap_size);
  print_bitmap(bitmap, bitmap_size);
  ASSERT_EQUAL(bitmap_sum(bitmap, bitmap_size), 0);
  for (size_t i = 0; i < bitmap_size; i++) {
    if (i % 2)
      set_bit(bitmap, i, 1);
  }
  print_bitmap(bitmap, bitmap_size);
  ASSERT_EQUAL(bitmap_sum(bitmap, bitmap_size), bitmap_size / 2);
  for (int i = 8; i < 16; i++) {
    toggle_bit(bitmap, i);
  }
  print_bitmap(bitmap, bitmap_size);
  ASSERT_EQUAL(bitmap_sum(bitmap, bitmap_size), bitmap_size / 2);
  for (int i = 16; i < bitmap_size; i++) {
    set_bit(bitmap, i, 1);
  }
  print_bitmap(bitmap, bitmap_size);
  ASSERT_EQUAL(bitmap_sum(bitmap, bitmap_size), (16/2) + (bitmap_size - 16));
  for (int i = 0; i < bitmap_size; i++) {
    set_bit(bitmap, i, 0);
  }
  ASSERT_EQUAL(bitmap_sum(bitmap, bitmap_size), 0);
  print_bitmap(bitmap, bitmap_size);
  for (int i = 0; i < bitmap_size; i++) {
    toggle_bit(bitmap, i);
  }
  ASSERT_EQUAL(bitmap_sum(bitmap, bitmap_size), bitmap_size);
  print_bitmap(bitmap, bitmap_size);
  for (int i = 0; i < bitmap_size; i++) {
    toggle_bit(bitmap, i);
  }
  ASSERT_EQUAL(bitmap_sum(bitmap, bitmap_size), 0);
  print_bitmap(bitmap, bitmap_size);
  return 0;
}

