#!/bin/bash
# Build script for engine
set echo on

mkdir -p ../bin

# Get a list of all the .c files.
cFilenames=$(find . -type f -name "*.c" -o -name "*.m")

# echo "Files : " $cFilenames

assembly="engine"
compilerFlags="-g -fdeclspec -fPIC -ObjC"
includeFlags="-Isrc -I$VULKAN_SDK/include"
linkerFlags="-shared -dynamiclib -install_name @rpath/lib$assembly.dylib -lobjc -framework AppKit -framework QuartzCore -lvulkan -L$VULKAN_SDK/lib"
defines="-D_DEBUG -DAEXPORT"
extension="dylib"

echo "Building $assembly..."
clang $cFilenames $compilerFlags -o ../bin/lib$assembly.$extension $defines $includeFlags $linkerFlags