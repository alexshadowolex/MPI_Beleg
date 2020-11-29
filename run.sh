#!/bin/bash

#Check for args
command_line_args=""
if [ -z "$2" ]
then
    echo "Using default test pictures"
    command_line_args="$1 files/test_pictures/paint1.jpg files/test_pictures/paint2.jpg"
else
    #Build an arg-String
    for arg; do
        command_line_args="${command_line_args} $arg"
    done
fi

#Run the program
echo "Run program"
./bin/main $command_line_args
#Save the return-value
ret=$?
extra_message=""
if [ $ret != 0 ]
then
    extra_message="with failure"
else
    extra_message="with success"
fi

#Echo a message stating the return of the program
echo "Finished running program $extra_message"
