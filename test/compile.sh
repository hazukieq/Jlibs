#!/bin/bash
_s=`pwd`
_s=${_s##*/}

if [ $_s == "Jlibs" ];then
        echo "current path is $_s"
else
        cd ~/cprojects/Jlibs
fi

./makeA.sh
cd ./test/

compile_f=${1##*/}
echo $compile_f
gcc ${compile_f} -ljtools -L../libs/ -o "${compile_f%.*}.o"

echo "input: $compile_f"
echo "output: ${compile_f%.*}.o"

c_="${compile_f%.*}.o"
echo -e "\nexecute:\r"
./$c_


