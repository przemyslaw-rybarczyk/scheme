#!/bin/sh

gcc -O2 -Wall -Wno-missing-braces *.c primitives/*.c -o scheme "${@:1}" &&
./scheme compiler.scm --compile compiler.sss
