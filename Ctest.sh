#!/bin/bash
if [[ $# -ge 1 ]];then
	gcc -g -fsanitize=address "$@" -o "${1%%.c}.o"
	clear
	"./${1%%.c}.o" >out.log
	result=$(cat out.log)
	rm out.log
	cur_time=$(date)
	printf "\033[0;31m----NEW_CTEST of %s " "$1"
	printf "%s----\033\n[0m" "$cur_time"
	printf "\033[0;32mexec:\n%s\n\033[0m" "$result"	
else
	echo "need to specify one input file..."
	exit
fi
