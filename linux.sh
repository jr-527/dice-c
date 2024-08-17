#!/bin/bash
# The executable .linux uses fgets for typing in input. From my testing, that
# works badly when you press the arrow keys or whatever, so this file is a
# wrapper that uses the bash "read" command.
if [ $# -ne 0 ]; then
    ./dice-linux "$@"
else
    while : ; do
        echo "Enter your input (q to quit):"
        read -e -p "" vari
        if [ "q" == "$vari" ]; then
            break
        fi
        ./dice-linux "$vari" || break
    done
fi
