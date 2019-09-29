#!/bin/bash

set -e

"${CC:-clang}" -O3 -Wall -Wconversion -Wstrict-prototypes *.c primitives/*.c unicode/unicode.c -o scheme "${@:1}"
cat compiler.scm macro.scm > _compiler.scm
./scheme _compiler.scm --compile compiler.sss
./scheme _compiler.scm --compile _compiler.sss
diff compiler.sss _compiler.sss || echo "Warning: compiler does not reproduce itself"
rm _compiler.scm
rm _compiler.sss
