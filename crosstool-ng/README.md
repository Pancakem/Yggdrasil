# Create arm cross-compilation tool under macos m1

## 1. Install `crosstool-NG`

````
brew install crosstool-ng
````

The current (2022-03-09) version of crosstool-NG installed by homebrew is 1.24.0_3, which needs some patches. If it is a new version, it may not be needed

### Patch 1: Replace `scripts/config.guess` file

````
cd /opt/homebrew/Cellar/crosstool-ng/1.24.0_3/share/crosstool-ng/scripts

./config.guess
````

If the returned value is not similar to `aarch64-apple-darwin21.3.0`, but starts with `arm`, it means that it needs to be replaced

````
curl https://raw.githubusercontent.com/crosstool-ng/crosstool-ng/master/scripts/config.guess > config.guess
````

### Patch 2: Add gcc patch

````
ls /opt/homebrew/Cellar/crosstool-ng/1.24.0_3/share/crosstool-ng/packages/gcc
````

See if there is a gcc version number you want to use, if not, go to the official website to download [https://github.com/crosstool-ng/crosstool-ng/tree/master/packages/gcc]

## 2. Install a gcc natively

A cross-compiled gcc also needs a gcc to compile it

````
brew install gcc
````

Pay attention to the gcc prefix and suffix on this machine, such as `aarch64-apple-darwin21-gcc-11` on my machine

Then the prefix is ​​`aarch64-apple-darwin21-`, the suffix is ​​`-11`

This should be consistent with `CT_BUILD_PREFIX`, `CT_BUILD_SUFFIX` in the `.config` file

## 3. Create a build environment

The operation of `crosstool-ng` requires a case-sensitive file system, which is insensitive by default under macOS, so create a new Volume

Use the disk utility that comes with the system, add an APFS volume, give it a name (such as Linux), and select 'APFS (case-sensitive)'

````
cd /Volumes/Linux
mkdir crosstool
cd crosstool
mkdir tarballs
mkdir x-tool
````

Copy the `.config` file to `/Volumes/Linux/crosstool`

## 4. Configure .config

Graphical modifications can be used

````
ct-ng menuconfig
````

You can also directly modify the `.config` file to change the required parameters.

Listed below are a few important

### CT_GMP_VERSION

The default GMP 6.1.2 will also have the above config.guess problem under m1, so choose a more recent age, here use `6.2.99-20220306144058`

### CT_BINUTILS_VERSION

The default is 2.32, but there are bugs (forgot), here we choose 2.38

### other

gcc version (CT_GCC_VERSION), linux kernel version (CT_LINUX_VERSION), glibc version (CT_GLIBC_VERSION) can be selected as needed

**Note 1: The version of gcc must ensure that the crosstool-ng patch exists, otherwise there will be many problems**

**Note 2: After the graphical modification, be sure to compare it with the previous file (.config.old), it will modify some default values, such as `CT_GMP_VERSION`**

## start operation

````
ulimit -n 2048
ct-ng build
````

After success, the file will be placed in the `x-tool` directory, or you can copy the directory to other places as a whole

have a test

````
/Volumes/Linux/crosstool/x-tool/bin/arm-unknown-linux-gnueabihf-gcc -v
````
