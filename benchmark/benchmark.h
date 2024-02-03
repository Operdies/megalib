#ifndef BENCHMARK_H
#define BENCHMARK_H
#include <time.h>
#include <bsd/sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


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

struct timespec spec_from_double(double t) {
  struct timespec spec;
  spec.tv_sec = (time_t)(t / 1e9);
  spec.tv_nsec = t - (double)spec.tv_sec * 1e9;
  return spec;
}

void readable_time(struct timespec time, char buf[30]){
  char *units[] = { "s", "ms", "μs", "ns" };
  if (time.tv_sec > 0) {
    snprintf(buf, 30, "%.3f %s", time.tv_sec + time.tv_nsec * 1e-9, units[0]);
  } else if (time.tv_nsec > 1e6) {
    snprintf(buf, 30, "%.3f %s", time.tv_nsec * 1e-6, units[1]);
  } else if (time.tv_nsec > 1e3) {
    snprintf(buf, 30, "%.3f %s", time.tv_nsec * 1e-3, units[2]);
  } else {
    snprintf(buf, 30, "%ld %s", time.tv_nsec, units[3]);
  }
}

static inline void print_summary(const char *benchstr, const struct timespec *m, int n) {
  struct timespec sum = {0};
  double mean, stddev, stderr, total_us;
  for (int i = 0; i < n; i++) {
    timespecadd(&sum, &m[i], &sum);
  }

  total_us = sum.tv_sec * 1e9 + sum.tv_nsec;
  mean = total_us / n;
  stddev = 0;
  for (int i = 0; i < n; i++) {
    double diff = (m[i].tv_sec * 1e9 + m[i].tv_nsec) - mean;
    stddev += diff * diff;
  }

  stderr = sqroot(stddev / n);
  
  char buf[100];
  char mean_s[30];
  char err_s[30];
  struct timespec mean_spec = spec_from_double(mean);
  struct timespec err_spec = spec_from_double(stderr);
  readable_time(mean_spec, mean_s);
  readable_time(err_spec, err_s);

  snprintf(buf, 100, " %s ± %s ", mean_s, err_s);
  print_centered(benchstr, buf);
  printf("%.*s\n", (int)strlen(benchstr), divider);                      
}
#define max_runs 10000000
#define min_runs 10
#define clamp(x, lo, hi) ((x) < (lo) ? (lo) : (x) > (hi) ? (hi) : (x))
#define benchmark(fname, ...)                                                 \
{                                                                             \
const char _m_testname[] = "    Benchmark " #fname "(" #__VA_ARGS__ ")    ";  \
print_divider(_m_testname);                                                   \
print_centered(_m_testname, " WARMING UP ");                                  \
struct timespec _m_start, _m_now, _m_elapsed;                                 \
clock_gettime(CLOCK_REALTIME, &_m_start);                                     \
int _m_runs = 0;                                                              \
while (1) {                                                                   \
  clock_gettime(CLOCK_REALTIME, &_m_now);                                     \
  timespecsub(&_m_now, &_m_start, &_m_elapsed);                               \
  if (_m_elapsed.tv_sec >= 3) break;                                          \
  (void)fname(__VA_ARGS__);                                                   \
  _m_runs++;                                                                  \
}                                                                             \
print_centered(_m_testname, " BENCHMARKING ");                                \
_m_runs = clamp(_m_runs, min_runs, max_runs);                                 \
struct timespec *_m_measurements = malloc(sizeof(struct timespec) * _m_runs); \
for (int i = 0; i < _m_runs; i++) {                                           \
  clock_gettime(CLOCK_REALTIME, &_m_start);                                   \
  (void)fname(__VA_ARGS__);                                                   \
  clock_gettime(CLOCK_REALTIME, &_m_now);                                     \
  timespecsub(&_m_now, &_m_start, &_m_measurements[i]);                       \
}                                                                             \
print_summary(_m_testname, _m_measurements, _m_runs);                         \
free(_m_measurements);                                                        \
}

#endif
