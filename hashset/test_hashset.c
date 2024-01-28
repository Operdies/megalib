#include "hashset.h"
#include <stdio.h>

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

int main(void) {
  return test_string_map();
}
