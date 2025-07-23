# M-VBA
Another gba emulator for miyoo, this is my attemp to make it work on miyoo

# How to build
You need docker, debian linux, arch linux
~~~
# if you first time build code, run this command to set up toolchain
docker pull miyoocfw/toolchain-shared-uclibc:latest

# to compile
docker run --volume ./:/src/ -it miyoocfw/toolchain-shared-uclibc:latest
cd src
make -f Makefile.miyoo -j $(nproc)
~~~
