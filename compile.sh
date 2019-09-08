#!/bin/bash

"${CC:-gcc}" -O3 -Wall -Wconversion *.c primitives/*.c -o scheme "${@:1}" &&
cat compiler.scm macro.scm > _compiler.scm &&
./scheme _compiler.scm --compile compiler.sss
rm _compiler.scm
