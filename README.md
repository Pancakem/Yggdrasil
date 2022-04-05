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

## Docker for cross-compilation
Docker has excellent cross-platform support/
This docker image `ev3dev/debian-stretch-cross` has a lot of
development tools pre-installed.

Preparing the docker image
`docker pull ev3dev/debian-stretch-cross`

Tag it with a shorter name
`docker tag ev3dev/debian-stretch-cross dev-env`
`docker run --rm -it -v <host-dir>:<working-container-dir> -w <working-container-dir> dev-env`

Small programs that do not depend on any hardware drivers can be run using using qemu:

Take for instance, the program below
```c
#include <stdio.h>

int main(void) { printf("Hello world\n");}
```

Compile
```sh
arm-linux-gnueabi-gcc -o hello hello.c
```

Run
```sh
qemu-arm-static -L /usr/arm-linux-gnueabi hello
```
