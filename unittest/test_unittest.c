#include <stdio.h>
#include "unittest.h"

int main(void){
  #define worldstr "world"
  ASSERT(1);
  ASSERT_EQ(1, 1);
  ASSERT_STREQ("Hello", "Hello");
  ASSERT_STREQ("world", worldstr);
  return 0;
}
