#!/bin/sh
#Compile the source
DEBUG=""
if [ ! -z "$1" ]
then
    DEBUG="-DDEBUG"
fi

gcc -omain src/main.c src/functions.c -lm $DEBUG

#Move binaries to bin-folder
mv main bin

#Success
tmp=""
if [ ! -z $DEBUG ]
then
    tmp="with Debug option"
fi
echo "Compiled successfully $tmp"