#!/bin/sh

gcc -O2 -Wall *.c primitives/*.c -o scheme "${@:1}" &&
./scheme compiler.scm --compile compiler.sss
