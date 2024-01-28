#include "hashset.h"
#include <stdio.h>
#include <stddef.h>

#define LENGTH(X) (sizeof(X) / sizeof(X[0]))
#define LOG(...) printf(__VA_ARGS__)

char *strfmt(kvp_t kvp) {
  static char fmtbuf[256];
  snprintf(fmtbuf, 256, "%10s => %s", kvp.key.string, kvp.value.string);
  return fmtbuf;
}

int test_string_map() {
  hashset h;
  mk_hashset(&h, hash_string, hashmap_strcmp, 7);
  hashset_print(&h, strfmt);
  kvp_t test_keys[] = {
    { .key =  { "123" } , .value = { "456" } },
    { .key =  { "123" } , .value = { "456" } },
    { .key =  { "456" } , .value = { "789" } },
    { .key =  { "789" } , .value = { "123" } },
    { .key =  { "abc" } , .value = { "def" } },
    { .key =  { "def" } , .value = { "ghi" } },
    { .key =  { "ghi" } , .value = { "jkl" } },
    { .key =  { "jkl" } , .value = { "mno" } },
    { .key =  { "mno" } , .value = { "pqr" } },
    { .key =  { "pqr" } , .value = { "stu" } },
    { .key =  { "stu" } , .value = { "vwx" } },
    { .key =  { "vwx" } , .value = { "yz" } },
  };

  for (size_t i = 0; i < LENGTH(test_keys); i++) {
    if (hashset_add(&h, test_keys[i])) {
      LOG("Added %s\n", test_keys[i].key.string);
    } else {
      LOG("Failed to add %s\n", test_keys[i].key.string);
    }
  }
  hashset_print(&h, strfmt);

  hashset_value removed;
  kvp_t overrider = { .key = { "123" }, .value = { "overridden" } };
  if (hashset_set(&h, overrider, &removed)) {
    LOG("Override key: %s\n", strfmt((kvp_t) { .key = overrider.key, .value = removed }));
  } else {
    LOG("Failed to override with %s\n", strfmt(overrider));
  }
  hashset_print(&h, strfmt);

  hashset_key keys_to_remove[] = {
    { "456"},
    { "789"},
    { "abc"},
    { "jkl"},
    { "mno"},
    { "pqr"},
  };

  for (size_t i = 0; i < LENGTH(keys_to_remove); i++) {
    if (hashset_remove(&h, keys_to_remove[i], &removed)) {
      LOG("Removed %s\n", strfmt((kvp_t) { .key = keys_to_remove[i], .value = removed }));
    } else {
      LOG("Failed to remove %s\n", strfmt((kvp_t) { .key = keys_to_remove[i], .value = removed }));
    }
  }
  hashset_print(&h, strfmt);

  for (size_t i = 0; i < LENGTH(test_keys); i++) {
    if (hashset_add(&h, test_keys[i])) {
      LOG("Added %s\n", test_keys[i].key.string);
    } else {
      LOG("Failed to add %s\n", test_keys[i].key.string);
    }
  }
  hashset_print(&h, strfmt);

  destroy_hashset(&h);
  return 0;
}

// hashing function which colliddes a lot
size_t hash_integer_bad(hashset_key key) {
  return key.integer;
}

int test_intmap() {
  hashset h;
  i64 start = 999;
  i64 end = 9999999;
  i64 count = end - start;
  mk_hashset(&h, hash_integer, NULL, 0);
  for (int n = 0; n < 10; n++) {
    { // add a bunch of keys
      for (i64 i = start; i < end; i++) {
        hashset_add(&h, (kvp_t) { .key = { .integer = i }, .value = { .integer = i * i } });

        { // read back the key we just added
          hashset_value value;
          if (!hashset_get(&h, (hashset_key) { .integer = i }, &value)) {
            printf("Failed to get %lld\n", i);
            return 0;
          }
          if (value.integer != i * i) {
            printf("Expected %lld, got %lld\n", i * i, value.integer);
            return 0;
          }
        }
      }

      if (h.count != count) {
        printf("Expected %lld elements, got %zu\n", count, h.count);
        return 0;
      }
    }

    { // override all the keys
      for (i64 i = start; i < end; i++) {
        i64 expected = i * i;
        hashset_value removed;
        if (!hashset_set(&h, (kvp_t) { .key = { .integer = i }, .value = { .integer = expected * i } }, &removed)) {
          printf("Failed to set %lld\n", i);
          return 0;
        }
        if (removed.integer != expected) {
          printf("Expected %lld, got %lld\n", expected, removed.integer);
          return 0;
        }
      }
    }

    { // remove the overriden keys
      for (i64 i = start; i < end; i++) {
        i64 expected = i * i * i;
        hashset_value removed;
        if (!hashset_remove(&h, (hashset_key) { .integer = i }, &removed)) {
          printf("Failed to remove %lld\n", i);
          return 0;
        }
        if (removed.integer != expected) {
          printf("Expected %lld, got %lld\n", expected, removed.integer);
          return 0;
        }
      }
      if (h.count != 0) {
        printf("Expected 0 elements, got %zu\n", h.count);
        return 0;
      }
    }
  }
  destroy_hashset(&h);

  return 1;
}

int main(void) {
  return !test_intmap();
  // return test_string_map();
}
