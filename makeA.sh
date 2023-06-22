#!/bin/bash
echo "generating a static library for the current directory..."
all_c=`ls *.c`
mkdir build
gcc -c ${all_c[@]}
mv *.o ./build
all_o=`ls build/*.o`
ar -r ./libs/libjtools.a ${all_o[@]}
rm build -r
echo "generated a static library for the current directory successfully."
