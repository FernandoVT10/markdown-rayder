#!/bin/bash

mkdir -p build
gcc -Wall -Werror -o ./build/main main.c -I./raylib-5.5/include -L./raylib-5.5/lib/ -l:libraylib.a -lm
