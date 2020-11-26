#!/bin/sh
#Compile the source
DEBUG=""
if [ ! -z "$1" ]
then
    DEBUG="-DDEBUG"
fi


#Success
tmp=""
if [ ! -z $DEBUG ]
then
    tmp="with Debug option"
fi

gcc -omain src/main.c src/list.c src/functions.c -lm $DEBUG

#Move binaries to bin-folder
mv main bin

echo "Finished compiling $tmp"
echo "Check gcc-output for info"