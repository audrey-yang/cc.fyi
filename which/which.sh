#!/bin/bash

IFS=":"

for prog in "$@"; do
    found=0
    for path in $PATH; do
        if [ -f "$path/$prog" ]; then
            echo "$path/$prog"
            found=1
            break
        fi
    done
    if [ $found = 0 ]; then
        echo $prog not found
    fi
done
