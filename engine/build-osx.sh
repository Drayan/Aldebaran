#!/bin/bash
# Build script for engine
set echo on

mkdir -p ../bin

# Get a list of all the .c files.
cFilenames=$(find . -type f -name "*.c")

# echo "Files : " $cFilenames

assembly="engine"
compilerFlags="-g -shared -fdeclspec -fPIC"
includeFlags="-Isrc -I$VULKAN_SDK/include"
linkerFlags="-lvulkan -L$VULKAN_SDK/lib"
defines="-D_DEBUG -DAEXPORT"
extension="so"

echo "Building $assembly..."
clang $cFilenames $compilerFlags -o ../bin/lib$assembly.$extension $defines $includeFlags $compilerFlags