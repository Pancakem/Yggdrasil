#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

uint64_t modulus = 1ULL << 31; // 2^31

uint64_t loop(uint64_t N, uint64_t S, uint64_t P, uint64_t Q) {
  for (uint64_t i = 0; i < N; i++) {
    S = (S * P + Q) % modulus;
  }
  return S;
}

static uint64_t modulus_static = 1ULL << 31; // 2^31

uint64_t loop_static(uint64_t N, uint64_t S, uint64_t P, uint64_t Q) {
  for (uint64_t i = 0; i < N; i++) {
    S = (S * P + Q) % modulus_static;
  }
  return S;
}

int main(int argc, char *argv[]) {
  if (argc != 5) {
    return 1;
  }

  uint64_t N = strtoll(argv[1], NULL, 10);
  uint64_t S = strtoll(argv[2], NULL, 10);
  uint64_t P = strtoll(argv[3], NULL, 10);
  uint64_t Q = strtoll(argv[4], NULL, 10);
  printf("N: %" PRIu64 ", S: %" PRIu64 ", P: %" PRIu64 ", Q: %" PRIu64 "\n", N,
         S, P, Q);

  struct timespec begin, end;
  clock_gettime(CLOCK_MONOTONIC_RAW, &begin);
  printf("result (non-static): %" PRIu64 "\n", loop(N, S, P, Q));
  clock_gettime(CLOCK_MONOTONIC_RAW, &end);
  printf(" took %lld milliseconds\n",
         (end.tv_nsec - begin.tv_nsec) / 1000000LL +
             (end.tv_sec - begin.tv_sec) * 1000LL);

  clock_gettime(CLOCK_MONOTONIC_RAW, &begin);
  printf("result (static): %" PRIu64 "\n", loop_static(N, S, P, Q));
  clock_gettime(CLOCK_MONOTONIC_RAW, &end);
  printf(" took %lld milliseconds\n",
         (end.tv_nsec - begin.tv_nsec) / 1000000LL +
             (end.tv_sec - begin.tv_sec) * 1000LL);

  return 0;
}
