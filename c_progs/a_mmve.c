#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void *mmove(void *dest, const void *src, unsigned int n) {
  char *pDst = (char *)dest;
  const char *pSrc = (const char *)src;

  char *tmp = (char *)malloc(sizeof(char) * n);
  if (tmp == NULL)
    return NULL;

  else {
    unsigned int i = 0;

    for (i = 0; i < n; ++i)
      *(tmp + i) = *(pSrc + i);

    for (i = 0; i < n; ++i)
      *(pDst + i) = *(tmp + i);

    free(tmp);
  }

  return dest;
}

int main(void) {

  char dest[] = "Aticleworld";
  const char src[] = "Amlendra";

  char dest1[] = "Aticleworld";
  const char src1[] = "Amlendra";

  struct timespec begin, end;
  clock_gettime(CLOCK_MONOTONIC_RAW, &begin);
  printf("result (std lib): %s\n", (char *)memmove(dest, src, 5));
  clock_gettime(CLOCK_MONOTONIC_RAW, &end);
  printf(" took %lld milliseconds\n",
         (end.tv_nsec - begin.tv_nsec) / 1000000LL +
             (end.tv_sec - begin.tv_sec) * 1000LL);

  clock_gettime(CLOCK_MONOTONIC_RAW, &begin);
  printf("result (self): %s\n", (char *)mmove(dest1, src1, 5));
  clock_gettime(CLOCK_MONOTONIC_RAW, &end);
  printf(" took %lld milliseconds\n",
         (end.tv_nsec - begin.tv_nsec) / 1000000LL +
             (end.tv_sec - begin.tv_sec) * 1000LL);

  printf("After std library memmove %s %s\n", dest, src);
  printf("After my memmove %s %s\n", dest1, src1);
}
