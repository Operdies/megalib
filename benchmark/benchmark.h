#ifndef BENCHMARK_H
#define BENCHMARK_H
#include <sys/time.h>
#include <stdio.h>
#include <string.h>


// avoid linking math just for a sqrt function
// https://stackoverflow.com/questions/29018864/any-way-to-obtain-square-root-of-a-number-without-using-math-h-and-sqrt
static inline double sqroot(double square)
{
  double MINDIFF = 2.25e-308;
  double root=square/3, last, diff=1;
  if (square <= 0) return 0;
  do {
    last = root;
    root = (root + square / root) / 2;
    diff = root - last;
  } while (diff > MINDIFF || diff < -MINDIFF);
  return root;
}

static const char divider[] = "================================================================================";
static inline void print_divider(const char *benchstr) {
  printf("%.*s\n", (int)strlen(benchstr), divider);                      
  puts(benchstr);                                                        
}

static inline size_t utf8_strlen(const char *s){
  size_t i = 0, j = 0;
  while (s[i]) {
    if ((s[i] & 0xC0) != 0x80) j++;
    i++;
  }
  return j;
}

static inline void print_centered(const char *centered_in, const char *value){
  int len = utf8_strlen(centered_in);
  int value_len = utf8_strlen(value);
  int pad = (len - value_len) / 2;
  printf("%.*s%s%.*s\n", pad, divider, value, len - pad - value_len, divider);
}

static inline void print_summary(const char *benchstr, const struct timeval *m, int n) {
  struct timeval sum = {0};
  double mean, stddev, stderr, total_us;
  for (int i = 0; i < n; i++) {
    timeradd(&sum, &m[i], &sum);
  }

  total_us = sum.tv_sec * 1e6 + sum.tv_usec;
  mean = total_us / n;
  stddev = 0;
  for (int i = 0; i < n; i++) {
    double diff = (m[i].tv_sec * 1e6 + m[i].tv_usec) - mean;
    stddev += diff * diff;
  }

  stderr = sqroot(stddev / n);

  char buf[100];
  snprintf(buf, 100, " %.0f μs ± %.0f μs ", mean, stderr);
  print_centered(benchstr, buf);
  printf("%.*s\n", (int)strlen(benchstr), divider);                      
}
#define max_runs 100000
#define min_runs 10
#define clamp(x, lo, hi) ((x) < (lo) ? (lo) : (x) > (hi) ? (hi) : (x))
#define benchmark(fname, ...)                                                 \
{                                                                             \
const char testname[] = "    Benchmark " #fname "(" #__VA_ARGS__ ")    ";     \
print_divider(testname);                                                      \
print_centered(testname, " WARMING UP ");                                     \
struct timeval start, now, elapsed;                                           \
gettimeofday(&start, NULL);                                                   \
int runs = 0;                                                                 \
while (1) {                                                                   \
  gettimeofday(&now, NULL);                                                   \
  timersub(&now, &start, &elapsed);                                           \
  if (elapsed.tv_sec >= 1) break;                                             \
  (void)fname(__VA_ARGS__);                                                   \
  runs++;                                                                     \
}                                                                             \
print_centered(testname, " BENCHMARKING ");                                   \
runs = clamp(runs, min_runs, max_runs);                                       \
struct timeval measurements[max_runs];                                        \
for (int i = 0; i < runs; i++) {                                              \
  gettimeofday(&start, NULL);                                                 \
  (void)fname(__VA_ARGS__);                                                   \
  gettimeofday(&now, NULL);                                                   \
  timersub(&now, &start, &measurements[i]);                                   \
}                                                                             \
print_summary(testname, measurements, runs);                                  \
}

#endif
