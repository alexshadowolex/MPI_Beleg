#!/bin/bash

evaluation_parts=(
    "Reading File Data"
    "Calculating Motion Vectors TOTAL"
    "Encoding Files"
    "Ending Program"
    "Other"
    "Total Program"
)

list_evaluation=()
next_evaluation_index=0

run_and_evaluate()
{
    amount_processes="$1"
    amount_vectors="$2"

    programm_output=$(mpiexec -n $amount_processes ./bin/main $amount_vectors "files/test_pictures/serienbild1.jpg" "files/test_pictures/serienbild2.jpg")

    new_evaluation_list=""
    iterator=0

    for part in "${evaluation_parts[@]}"
    do
        milliseconds=$(echo "$programm_output" | grep -Po "$part.*: \K.*ms" | grep -o "[0-9]\+\\.[0-9]\+")
        seconds=$(echo "$programm_output" | grep -Po "$part.*: .*\\(=\K.*s" | grep -o "[0-9]\+\\.[0-9]\+")
        
        new_evaluation_list+="$milliseconds|$seconds;"
        let iterator=$iterator+1
    done
    list_evaluation[$next_evaluation_index]=$new_evaluation_list
    let next_evaluation_index=$next_evaluation_index+1
}

# Main

RANGE_START_PROCESSORS=1

range_end_processors=$1
distance_vectors=$2

if [ -z "$1" -o -z "$2" ]
then
    echo "Please provide a command line argument for each \"Range End Processors\" (> $RANGE_START_PROCESSORS) and \"Distance Motion Vectors\" (>= 0)"
    exit 1
fi

if [ $1 -lt $RANGE_START_PROCESSORS -o $2 -lt 0 ]
then
    echo "Please provide a valid value for each command line argument:"
    echo "Range End Processors > $RANGE_START_PROCESSORS"
    echo "Range End Vectors >= 0"
    exit 1
fi

echo ""
echo "Testing with Values: Range Processors: $RANGE_START_PROCESSORS-$1 | Distance Motion Vectors: $distance_vectors"

# Reference values for each vector amount
for (( iterator_processor=$RANGE_START_PROCESSORS; iterator_processor <= $range_end_processors; iterator_processor++ ))
do
    echo "Test with Amount Processes = $iterator_processor and Distance Motion Vectors = $distance_vectors"
    run_and_evaluate "$iterator_processor" "$distance_vectors"
done

reference_times=()
for (( iterator_processors=$RANGE_START_PROCESSORS; iterator_processors<=$range_end_processors; iterator_processors++ ))
do
    if [ $iterator_processors -eq $RANGE_START_PROCESSORS ]
    then
        echo "=========================================================="
        echo "Evaluation for Usage with Motion Vector Distance of $distance_vectors"
        echo ""
    fi
    echo "Used times with $iterator_processors Processors"
    
    values=${list_evaluation[$iterator_processors - $RANGE_START_PROCESSORS]}
    for (( iterator_values=1; iterator_values<${#evaluation_parts[@]}+1; iterator_values++ ))
    do
        combined_values=$(echo "$values" | cut -d";" -f$iterator_values)
        milliseconds=$(echo "$combined_values" | cut -d"|" -f1)
        seconds=$(echo "$combined_values" | cut -d"|" -f2)
        speed_up=""
        speed_up_string=""
        if [ $iterator_processors -eq $RANGE_START_PROCESSORS -o -z "$iterator_processors" ]
        then
    	    reference_times[$iterator_values-1]=$milliseconds
        else
            echo "$(echo "${reference_times[$iterator_values-1]} != 0" |bc -l)"
            if [ $(echo "${reference_times[$iterator_values-1]} == 0" |bc -l) ]
            then
                echo "Speedup 0"
                speed_up=0
            else
                echo "Speedup calced"
                speed_up=$(echo "(${reference_times[$iterator_values-1]}-milliseconds)*100/${reference_times[$iterator_values-1]}" | bc)
            fi
            speed_up_string="====> Speed up: $speed_up%"
        fi
        echo "    ${evaluation_parts[$iterator_values-1]}: $milliseconds ms (= $seconds s) $speed_up_string"
    done
done