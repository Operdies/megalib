#include "hashset.h"
#include "../benchmark/benchmark.h"
#include <stdio.h>
#include <stddef.h>
#include <sys/time.h>

#define LENGTH(X) (sizeof(X) / sizeof(X[0]))
#define LOG(...) printf(__VA_ARGS__)

char *strfmt(kvp_t kvp) {
  static char fmtbuf[256];
  snprintf(fmtbuf, 256, "%10s => %s", kvp.key.string, kvp.value.string);
  return fmtbuf;
}

int test_string_map() {
  hashset h;
  mk_hashset(&h, hash_string, hashset_strcmp, 7);
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

// hashing function which collides a lot
size_t hash_integer_bad(hashset_key key) {
  return key.integer;
}

#define TIME(stmnt)                                                                        \
do {                                                                                       \
  struct timeval ___STARTTIME___;                                                          \
  gettimeofday(&___STARTTIME___, NULL);                                                    \
  stmnt;                                                                                   \
  struct timeval ___ENDTIME___;                                                            \
  gettimeofday(&___ENDTIME___, NULL);                                                      \
  timersub(&___ENDTIME___, &___STARTTIME___, &___ENDTIME___);                              \
  printf("%2zus %3zu ms\n%s\n", ___ENDTIME___.tv_sec,                                      \
      ___ENDTIME___.tv_usec / 1000, #stmnt);                                               \
}                                                                                          \
while (0); 


int test_hashset(i64 start, i64 end, i64 increment, hashfunc_t fn) {
  int res = 1;
  i64 count = (end - start - 1) / increment + 1;
  hashset h2;
  mk_hashset(&h2, fn, NULL, 0);
  hashset *h = &h2;
  for (int n = 0; n < 10; n++) {
    for (i64 i = start; i < end; i += increment) {
      hashset_key key = { .integer = i };
      if (!hashset_add(h, (kvp_t) { .key = key, .value = { .integer = i * i } })) {
        printf("Failed to add %lld\n", i);
        res = 0;
        goto end;
      }

      { // read back the key we just added
        hashset_value value;
        if (!hashset_get(h, key, &value)) {
          printf("Failed to get %lld\n", i);
          res = 0;
          goto end;
        }
        if (value.integer != i * i) {
          printf("Expected %lld, got %lld\n", i * i, value.integer);
          res = 0;
          goto end;
        }
      }
    }

    if (h->count != count) {
      printf("Expected %lld elements, got %zu\n", count, h->count);
      res = 0;
      goto end;
    }

    for (i64 i = start; i < end; i += increment) {
      i64 expected = i * i;
      hashset_key key = { .integer = i };
      hashset_value removed;
      if (!hashset_remove(h, key, &removed)) {
        printf("Failed to remove %lld/%lld\n", i, end);
        res = 0;
        goto end;
      }
      if (removed.integer != expected) {
        printf("Expected %lld, got %lld\n", expected, removed.integer);
        res = 0;
        goto end;
      }
    }
    if (h->count != 0) {
      printf("Expected 0 elements, got %zu\n", h->count);
      res = 0;
      goto end;
    }
  }
end:
  destroy_hashset(h);
  return res;
}

size_t bad(hashset_key key) {
  return key.integer;
}

size_t ideal(hashset_key key) {
  return key.integer * 2;
}

size_t generic(hashset_key key) {
  return (key.integer * 2654435761) % 4294967296;
}

int main(void) {
  i64 start = 169;
  i64 end = 123456;
  i64 increment = 7;
  benchmark(test_hashset, start, end, increment, bad);
  benchmark(test_hashset, start, end, increment, ideal);
  benchmark(test_hashset, start, end, increment, generic);
  // TIME(test_hashset(999, 9999999));
  return 0;
}
