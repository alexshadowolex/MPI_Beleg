#!/bin/sh
#Write debug String, if given with $1
DEBUG=""
if [ ! -z "$1" ]
then
    DEBUG="-DDEBUG"
fi


#Modify output String, when Debug options are enabled
tmp=""
if [ ! -z $DEBUG ]
then
    tmp="with Debug option"
fi

#Compile the source
gcc -omain src/main.c src/list.c src/functions.c -lm $DEBUG

#Move binaries to bin-folder
mv main bin

echo "Finished compiling $tmp"
echo "Check gcc-output for info"
