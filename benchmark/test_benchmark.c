#include "benchmark.h"

int my_func(size_t a, size_t b, size_t c){
  size_t result = 0;
  for (size_t i = 0; i < a*b*c; i++)
    result += i;
  return result;
}

int main(void) {
  benchmark(my_func, 10, 20, 30);
  benchmark(my_func, 100, 200, 300);
}
