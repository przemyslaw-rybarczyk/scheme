#!/bin/sh

gcc -O2 -Wall -Wno-missing-braces *.c primitives/*.c -o scheme "${@:1}"
./scheme_compiler --compile < compiler.scm
mv -f compiled.sss compiler.sss
