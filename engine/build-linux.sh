#!/bin/bash
# Build script for engine
set echo on

mkdir -p ../bin

# Get a list of all the .c files.
cFilenames=$(find . -type f -name "*.c")

# echo "Files : " $cFilenames

assembly="engine"
compilerFlags="-g -shared -fdeclspec -fPIC"
includeFlags="-Isrc"
linkerFlags="-lvulkan -lxcb -lX11 -lX11-xcb -lxkbcommon -L/usr/lib64/X11"
defines="-D_DEBUG -DAEXPORT"
extension="so"

echo "Building $assembly..."
clang $cFilenames $compilerFlags -o ../bin/lib$assembly.$extension $defines $includeFlags $linkerFlags