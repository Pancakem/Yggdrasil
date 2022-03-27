# Yggdrasil

## Useful Macros
Wrapping malloc and free for debug
```C
static void *malloc_debug(size_t n, unsigned long line) {
  printf("allocating memory %s:%lu\n", __FILE__, line);
  return malloc(n);
}

static void free_debug(void *p, unsigned long line) {
  printf("freeing memory %s:%lu\n", __FILE__, line);
  free(p);
}

#define MALLOC(n) malloc_debug(n, __LINE__);

#define FREE(n) free_debug(n, __LINE__);
```
