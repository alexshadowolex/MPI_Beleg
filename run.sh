#!/bin/bash

#Check for args
command_line_args=""
if [ -z "$1" ]
then
    echo "Please give an amount of processes as second argument"
else
    amount_processes="$1"
    if [ -z "$3" ]
    then
        echo "Using default test pictures"
        command_line_args="$2 files/test_pictures/paint1.jpg files/test_pictures/paint2.jpg"
    else
        #Build an arg-String
        command_line_args="${@:2}"
    fi

    #Run the program
    echo "Run program"
    # echo "mpiexec -n $amount_processes ./bin/main $command_line_args"
    mpiexec -n $amount_processes ./bin/main $command_line_args
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
fi
