typedef int (*compare_func_t)(const void* left, const void* right);
typedef struct {
  compare_func_t comparer;
}
