#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "functions.h"
#if __has_include(<mpi.h>)
#   include <mpi.h>
#endif

int main(int argc, char ** argv){

    // Take time for the whole program execution
    struct timeval total_end_time, total_start_time;
    gettimeofday(&total_start_time, NULL);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &amount_processes);

    // Some sanity checks
    if(argc <= 3){
        print_timestamp();
        printf("Not enough args! Usage: %s <distanze_motion_vector_search> <ref_picture> <picture 1> (optional: more picturesult)\n", argv[0]);
        MPI_Abort(MPI_COMM_WORLD ,EXIT_FAILURE);
    }

    int amount_files = argc - 2;
    int distanze_motion_vector_search = atoi(argv[1]);

    if(distanze_motion_vector_search < 0){
        print_timestamp();
        printf("Given distance for the motion vector search %i is not >= 0. Please provide a value greater or equal zero\n", distanze_motion_vector_search);
        MPI_Abort(MPI_COMM_WORLD ,EXIT_FAILURE);
    }

    // Initilize all used MPI_Datatypes
    init_mpi_data_types();
    
    // List saves all time evalutation through out the program and prints them at the end 
    time_evaluation_list = create_list();
    
    time_printf(("Amount Processes: %i\n", amount_processes));

    // List holds all data for each given file in struct tFile_data
    tList * file_data_list = create_list();

    time_printf(("Starting to read %i files\n", amount_files));

    struct timeval read_start_time, read_end_time;
    gettimeofday(&read_start_time, NULL);
    
    // Get the file data and store it in an array
    int iterator_files_read;
    for(iterator_files_read = 0; iterator_files_read < amount_files; iterator_files_read++) {
        // First arg is the distance_motion_vector_search 
        char * tmp_file_name = argv[iterator_files_read + 2];
        tFile_data * tmp_data = read_picture(tmp_file_name);
        time_printf(("Reading file %s finished!\n", tmp_data->file_name));
        xprintf(("Picture_width: %i | Picture_Height: %i\n\n", tmp_data->width, tmp_data->height));
        append_element(file_data_list, tmp_data);
    }

    gettimeofday(&read_end_time, NULL);
    add_to_evaluation_list("Reading File Data", read_start_time, read_end_time, -1.0);

    time_printf(("Finished reading %i files\n", amount_files));

    // This list is holding lists of tMacro_Block_SAD. At index 0, file_data_list[0] and file_data_list[1] are compared, 
    // at index 1 file_data_list[0] and file_data_list[2] are compared etc.
    tList * list_compared_pictures;
    if(rank == MASTER_RANK){
        list_compared_pictures = create_list();
    }

    time_printf(("Starting to calculate the motion vectors\n"));

    struct timeval calc_start_time, calc_end_time;
    gettimeofday(&calc_start_time, NULL);
    double time_for_calculation = 0.0;

    // When only one process exists, there is no master rank needed
    if(amount_processes == 1){
        int iterator_files;
        for(iterator_files = 0; iterator_files < amount_files - 1; iterator_files++){
            int range[2];
            get_range(range, get_amount_motion_vectors(distanze_motion_vector_search));
            char * file_name = ((tFile_data *) get_element(file_data_list, iterator_files + 1)->item)->file_name;

            time_printf(("Calculating motionvectors for number %i; picture-name: %s\n", (iterator_files + 1), file_name));

            append_element(
                list_compared_pictures, 
                calc_SAD_values(
                    (tFile_data *) get_element(file_data_list, 0)->item,
                    (tFile_data *) get_element(file_data_list, iterator_files + 1)->item,
                    distanze_motion_vector_search,
                    range[0],
                    range[1]
                )
            );
        }
    } else {
        // If there are at least 2 processes, the process with rank equal MASTER_RANK will collect and compare the data from all other ranks
        if(rank == MASTER_RANK ){
            // MASTER Tasks: Receive all values from each worker and evaluate and save them
            int iterator_files;
            // Iterate over all files
            for(iterator_files = 1; iterator_files < amount_files; iterator_files++){
                int iterator_macro_blocks;
                // Create for each file a list of tMacro_Block_SAD
                tList * tmp_macro_block_list = create_list();
                char * file_name = ((tFile_data *) get_element(file_data_list, iterator_files)->item)->file_name;

                time_printf(("Calculating motionvectors for number %i; picture-name: %s\n", iterator_files, file_name));

                // Iterate over all macro blocks
                for(iterator_macro_blocks = 0; iterator_macro_blocks < get_amount_macro_blocks((tFile_data *) get_element(file_data_list, 0)->item); iterator_macro_blocks++){
                    int iterator_amount_processes;
                    float current_minimal_SAD = __INT_MAX__ / 2;
                    tPixel_index current_best_motion_vector;

                    // Iterate over all processes
                    for(iterator_amount_processes = 1; iterator_amount_processes < amount_processes; iterator_amount_processes++){
                        tTMP_Macro_Block_SAD buffer;

                        // If we are in the first iteration, only receive a message from the first rank, which always checks the 0-vector
                        // so we guarantee that this vector is prioritzed (e.g. there are several vectors with SAD = 0, so it gets random chosen)
                        struct timeval calc_send_start_time, calc_send_end_time;
                        gettimeofday(&calc_send_start_time, NULL);
                        if(iterator_amount_processes == 1){
                            MPI_Recv(&buffer, 1, MPI_tMacro_Block_SAD, iterator_amount_processes, iterator_macro_blocks, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        } else {
                            // if not, the order doesn't matter
                            MPI_Recv(&buffer, 1, MPI_tMacro_Block_SAD, MPI_ANY_SOURCE, iterator_macro_blocks, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        }
                        gettimeofday(&calc_send_end_time, NULL);
                        time_for_calculation += calculate_time_difference(calc_send_start_time, calc_send_end_time);

                        // Evaluatue like in calc_SAD_values
                        if(buffer.value_SAD < current_minimal_SAD){
                            current_minimal_SAD = buffer.value_SAD;
                            current_best_motion_vector.x_width = buffer.x_width;
                            current_best_motion_vector.y_height = buffer.y_height;
                            // Stop if the value is 0
                            if(current_minimal_SAD == 0){
                                break;
                            }
                        }
                    }
                    // Add the values to the list of this picture
                    tMacro_Block_SAD * tmp_new_entry = malloc(sizeof(tMacro_Block_SAD));
                    tmp_new_entry->motion_vector = current_best_motion_vector;
                    tmp_new_entry->value_SAD = current_minimal_SAD;

                    append_element(tmp_macro_block_list, tmp_new_entry);
                }
                // Add the list to the final list
                append_element(list_compared_pictures, tmp_macro_block_list);
            }
        } else {
            // WORKER Tasks: send all found values to Master
            int iterator_files;
            // Iterate over all files
            for(iterator_files = 1; iterator_files < amount_files; iterator_files++){
                int range[2];
                get_range(range, get_amount_motion_vectors(distanze_motion_vector_search));
                // Get all motion vectors data from one rank
                tList * tmp_list = calc_SAD_values(
                    (tFile_data *) get_element(file_data_list, 0)->item,
                    (tFile_data *) get_element(file_data_list, iterator_files)->item,
                    distanze_motion_vector_search,
                    range[0],
                    range[1]
                );
                int iterator_macro_block;
                // Send each macro-block's SAD value and motion vector to master, so he can compare all of them
                for(iterator_macro_block = 0; iterator_macro_block < tmp_list->size; iterator_macro_block++){
                    tTMP_Macro_Block_SAD buffer;
                    tMacro_Block_SAD * tmp = (tMacro_Block_SAD *) get_element(tmp_list, iterator_macro_block)->item;
                    buffer.value_SAD = tmp->value_SAD;
                    buffer.x_width = tmp->motion_vector.x_width;
                    buffer.y_height = tmp->motion_vector.y_height;
                    
                    MPI_Send(&buffer, 1, MPI_tMacro_Block_SAD, MASTER_RANK, iterator_macro_block, MPI_COMM_WORLD);
                }
                // list is not needed anymore
                delete_list(tmp_list);
            }
        }
    }

    gettimeofday(&calc_end_time, NULL);

    if(amount_processes > 1){    
        add_to_evaluation_list("Calculating Motion Vectors and Sending only", calc_start_time, calc_end_time, time_for_calculation);
        add_to_evaluation_list("Evaluating sent Values", calc_start_time, calc_end_time, calculate_time_difference(calc_start_time, calc_end_time) - time_for_calculation);
    }

    add_to_evaluation_list("Calculating Motion Vectors TOTAL", calc_start_time, calc_end_time, -1.0);

    time_printf(("Finished calculating the motion vectors\n"));

    time_printf(("Starting to encode all files\n"));

    struct timeval encode_start_time, encode_end_time;
    gettimeofday(&encode_start_time, NULL);

    int ret_value = EXIT_SUCCESS;
    if(rank == MASTER_RANK){
        // Only the master encodes the files
        ret_value = encode_files(file_data_list, list_compared_pictures);
    }

    if(ret_value == EXIT_FAILURE){
        if(rank == MASTER_RANK){
            end_programm(file_data_list, list_compared_pictures);
        }
        delete_list(time_evaluation_list);

        MPI_Abort(MPI_COMM_WORLD ,EXIT_FAILURE);
    }
    
    gettimeofday(&encode_end_time, NULL);
    add_to_evaluation_list("Encoding Files", encode_start_time, encode_end_time, -1.0);

    time_printf(("Finished encoding all files\n"));

    time_printf(("Starting to free all malloced data\n"));

    struct timeval ending_start_time, ending_end_time;
    gettimeofday(&ending_start_time, NULL);

    if(rank == MASTER_RANK){
        // Only the master free's all data
        end_programm(file_data_list, list_compared_pictures);
    }

    gettimeofday(&ending_end_time, NULL);
    add_to_evaluation_list("Ending Program", ending_start_time, ending_end_time, -1.0);

    time_printf(("Finished freeing all malloced data\n"));

    time_printf(("Finished running the program!\n\n"));
    
    gettimeofday(&total_end_time, NULL);

    double tmp_time_difference = 0.0;
    int iterator_time_difference;
    // calculate the time difference for Other (prints etc.)
    for(iterator_time_difference = 0; iterator_time_difference < time_evaluation_list->size; iterator_time_difference++){
        tmp_time_difference += ((tTime_evaluation *) get_element(time_evaluation_list, iterator_time_difference)->item)->time_difference;
    }
    tmp_time_difference = calculate_time_difference(total_start_time, total_end_time) - tmp_time_difference;
    
    add_to_evaluation_list("Other", total_start_time, total_end_time, tmp_time_difference);
    
    add_to_evaluation_list("Total Program", total_start_time, total_end_time, -1.0);

    time_printf(("Evaluating used time\n"));

    int iterator_time_evaluation;
    // Evaluate the used time
    if(rank == MASTER_RANK){
        for(iterator_time_evaluation = 0; iterator_time_evaluation < time_evaluation_list->size; iterator_time_evaluation++){
            tTime_evaluation * tmp = (tTime_evaluation *) get_element(time_evaluation_list, iterator_time_evaluation)->item;
            time_printf(("Time used for %-50s: %0.3f ms (= %0.3f s)\n", tmp->evaluation_for, tmp->time_difference, (tmp->time_difference / 1000) ));
        }
    }
    delete_list(time_evaluation_list);
    
    MPI_Finalize();

    exit(EXIT_SUCCESS);
}