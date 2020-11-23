#!/bin/bash

#Check for args
picture1=""
picture2=""
if [ -z "$1" ]
then
    echo "Using default test pictures"
    picture1="files/test_pictures/paint1.jpg"
    picture2="files/test_pictures/paint2.jpg"
else
    picture1="$1"
    picture2="$2"
fi

#Run the program
echo "Run program"
./bin/main $picture1 $picture2
echo "Finished running program"
