#!/bin/bash

set -e

"${CC:-clang}" -O3 -Wall -Wconversion -Wstrict-prototypes *.c primitives/*.c unicode/unicode.c bigint/*.c -o scheme "${@:1}"
./scheme compiler.scm macro.scm --compile compiler.sss
./scheme compiler.scm macro.scm --compile _compiler.sss
diff compiler.sss _compiler.sss || echo "Warning: compiler does not reproduce itself"
rm _compiler.sss
