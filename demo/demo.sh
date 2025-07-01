#!/bin/sh

rm -rf build/
mkdir build/
gcc -o build/demo demo.c cJSON.c -I. -ansi -Wall -Wextra -pedantic-errors -lcurl
./build/demo
