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
machinefile_option=""
machinefile_name=""

run_and_evaluate()
{
    amount_processes="$1"
    amount_vectors="$2"

    programm_output=$(mpiexec "$machinefile_option" "$machinefile_name" -n $amount_processes ./bin/main $amount_vectors "files/test_pictures/serienbild1.jpg" "files/test_pictures/serienbild2.jpg")

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

if [ ! -z "$3" ]
then
    machinefile_option="-f"
    machinefile_name="machinefile"
fi


amount_macro_blocks=10266

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

total_output=""
best_time=""
best_amount_processors=""
for (( iterator_processors=$RANGE_START_PROCESSORS; iterator_processors<=$range_end_processors; iterator_processors++ ))
do
    if [ "$iterator_processors" -eq "$RANGE_START_PROCESSORS" ]
    then
        echo "=========================================================="
        echo "Evaluation for Usage with Motion Vector Distance of $distance_vectors"
        echo ""
    fi
    echo "Used times with $iterator_processors Processors"
    
    values=${list_evaluation[$iterator_processors - $RANGE_START_PROCESSORS]}
    seconds_calculation=""
    milliseconds_calculation=""
    for (( iterator_values=1; iterator_values<${#evaluation_parts[@]}+1; iterator_values++ ))
    do
        combined_values=$(echo "$values" | cut -d";" -f"$iterator_values")
        milliseconds=$(echo "$combined_values" | cut -d"|" -f1)
        seconds=$(echo "$combined_values" | cut -d"|" -f2)
        if [ "${evaluation_parts[$iterator_values-1]}" == "Calculating Motion Vectors TOTAL" ]
        then
            if [ 1 -eq "$(echo "$seconds == 0" | bc -l)" ]
            then
                seconds_calculation=100
            else
                seconds_calculation=$seconds
            fi
            if [ 1 -eq "$(echo "$milliseconds == 0" | bc -l)" ]
            then
                milliseconds_calculation=100
            else
                milliseconds_calculation=$milliseconds
            fi
        fi

        if [ "${evaluation_parts[$iterator_values-1]}" == "Total Program" ]
        then
            if [ -z "$best_time" ] || [ 1 -eq $(echo "$best_time > $seconds" | bc -l) ]
            then
                best_time=$seconds
                best_amount_processors=$iterator_processors
            fi
        fi

        speed_up=""
        speed_up_string=""
        if [ "$iterator_processors" -eq $RANGE_START_PROCESSORS ] || [ -z "$iterator_processors" ]
        then
    	    reference_times[$iterator_values-1]=$milliseconds
        else
            if [ 1 -eq "$(echo "${reference_times[$iterator_values-1]} == 0" | bc -l)" ]
            then
                speed_up=0
            else
                speed_up="$(printf "%0.3f\n" "$(echo "((${reference_times[$iterator_values-1]}-$milliseconds)*100)/${reference_times[$iterator_values-1]}" | bc -l)")"
            fi
            speed_up_string="====> Speed up: $speed_up%"
        fi
        output_string="    $(printf "%-35s\n" "${evaluation_parts[$iterator_values-1]}"): $milliseconds ms (= $seconds s) $speed_up_string"
        echo "$output_string"
        total_output+="$output_string \n"
    done
    extra_info="    Macro Blocks per Second: $(printf "%0.3f" "$(echo "$amount_macro_blocks/$seconds_calculation" | bc -l)") | Macro Blocks per milliseconds: $(printf "%0.3f" "$(echo "$amount_macro_blocks/$milliseconds_calculation" | bc -l)")"
    echo "$extra_info"
    total_output+=" $extra_info\n"
done

extra_info2="Best time: $best_time s with $best_amount_processors processors"
echo "$extra_info2"
total_output+=" $extra_info2\n"

file_name="1-${range_end_processors}_${distance_vectors}"
add_iterator=1
test_value="files/logs/${file_name}.log"
while [ -e "$test_value" ]
do
    test_value="files/logs/${file_name}_nr_${add_iterator}.log"
    let add_iterator=$add_iterator+1
done

file_name="$test_value"

touch -a "$file_name"
echo "$total_output" >> "$file_name"