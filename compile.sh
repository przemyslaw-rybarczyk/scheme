#!/bin/bash

"${CC:-gcc}" -O3 -Wall -Wconversion *.c primitives/*.c -o scheme "${@:1}" &&
./scheme compiler.scm --compile compiler.sss
