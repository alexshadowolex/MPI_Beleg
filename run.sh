#!/bin/bash

#Check for args
command_line_args=""
if [ -z "$1" ]
then
    echo "Using default test pictures"
    command_line_args="files/test_pictures/paint1.jpg files/test_pictures/paint2.jpg"
else
    for arg; do
        command_line_args="${command_line_args} $arg"
    done
fi

#Run the program
echo "Run program"
./bin/main $command_line_args
ret=$?
extra_message=""
if [ $ret != 0 ]
then
    extra_message="with failure"
else
    extra_message="with success"
fi
echo "Finished running program $extra_message"