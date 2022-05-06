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

A simple program that prints out a human readable name of a type
```cpp
#if __has_include(<cxxabi.h>)
#include <cxxabi.h>
#include <memory>
#endif

#include <iostream>

template <typename T>
std::string demangle()
{
#if __has_include(<cxxabi.h>)
    int status = -1;
    std::unique_ptr<char, void(*)(void*)> demangled_name
    {
        abi::__cxa_demangle(typeid(std::remove_reference_t<T>).name(), NULL, NULL, &status),
        std::free
    };
    if(status == 0)
        return demangled_name.get();
    else
        return typeid(std::remove_reference_t<T>).name();
#else
    return typeid(std::remove_reference_t<T>).name();
#endif
}

namespace B {
    namespace {
        struct A {};
    };
};

int main() {
    std::cout << demangle<B::A>();
}

```

void pointer arithmetic
```
#include <stdio.h>
#include <stdbool.h>

int main(void) {

    int x = 5;
    void *p = &x;
    printf("%p\n", p);
    p++;
    printf("%p\n", p);
    // the above is UB.
    // Notice p is incremented by a byte
    // should not perform pointer arithmetic
    // on incomplete types which void is UB
}

```

I met this error `libxx.so undefined reference to xxx@glibc` when cross-compiling
something.
Fixed it by recompiling the shared library using the same compiler version
I had been using.
