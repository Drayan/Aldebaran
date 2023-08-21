#!/bin/bash
# Build script for testbed
set echo on

mkdir -p ../bin

# Get a list of all the .c files.
cFilenames=$(find . -type f -name "*.c")

# echo "Files : " $cFilenames

assembly="testbed"
compilerFlags="-g -fdeclspec -fPIC"
includeFlags="-Isrc -I../engine/src"
linkerFlags="-L../bin/ -L$VULKAN_SDK/lib -lengine -Wl,-rpath,."
defines="-D_DEBUG -DAIMPORT"

echo "Building $assembly..."
clang $cFilenames $compilerFlags -o ../bin/$assembly $defines $includeFlags $linkerFlags