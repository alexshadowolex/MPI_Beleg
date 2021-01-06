#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "functions.h"
#if __has_include(<mpi.h>)
#   include <mpi.h>
#endif

int main(int argc, char ** argv){

    struct timeval total_end_time, total_start_time;
    gettimeofday(&total_start_time, NULL);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &amount_processes);

    if(argc <= 3){
        time_printf(("Not enough args! Usage: %s <distanze_motion_vector_search> <ref_picture> <picture 1> (optional: more picturesult)\n", argv[0]));
        MPI_Abort(MPI_COMM_WORLD ,EXIT_FAILURE);
    }

    int i;
    int amount_files = argc - 2;
    int distanze_motion_vector_search = atoi(argv[1]);

    if(distanze_motion_vector_search < 0){
        time_printf(("Given distance for the motion vector search %i is not >= 0. Please provide a value greater or equal zero\n", distanze_motion_vector_search));
        MPI_Abort(MPI_COMM_WORLD ,EXIT_FAILURE);
    }

    init_mpi_data_types();
    
    time_evaluation_list = create_list();
    
    time_printf(("Amount Processes: %i\n", amount_processes));

    xprintf(("amount_files: %i\n\n", amount_files));
    tList * file_data_list = create_list();

    //Get the file data and store it in an array
    time_printf(("Starting to read %i files\n", amount_files));
    struct timeval read_start_time, read_end_time;
    gettimeofday(&read_start_time, NULL);
    for(i = 0; i < amount_files; i++) {
        char * tmp_file_name = argv[i + 2];
        tFile_data * tmp_data = read_picture(tmp_file_name);
        time_printf(("Reading file %s finished!\n", tmp_data->file_name));
        xprintf(("Picture_width: %i | Picture_Height: %i\n\n", tmp_data->width, tmp_data->height));
        append_element(file_data_list, tmp_data);
    }
    gettimeofday(&read_end_time, NULL);
    add_to_evaluation_list("Reading File Data", read_start_time, read_end_time, -1.0);
    time_printf(("Finished reading %i files\n", amount_files));

    //This list is holding lists of tMacro_Block_SAD. At index 0, file_data_list[0] and file_data_list[1] are compared, 
    //at index 1 file_data_list[0] and file_data_list[2] are compared etc.
    tList * list_compared_pictures = create_list();

    time_printf(("Starting to calculate the motion vectors\n"));
    struct timeval calc_start_time, calc_end_time;
    gettimeofday(&calc_start_time, NULL);
    if(rank == MASTER_RANK && amount_processes > 1){
        int iterator_files;
        for(iterator_files = 0; iterator_files < amount_files - 1; iterator_files++){
            int iterator_macro_blocks;
            tList * tmp_macro_block_list = create_list();
            for(iterator_macro_blocks = 0; iterator_macro_blocks < get_amount_macro_blocks((tFile_data *) get_element(file_data_list, 0)->item); iterator_macro_blocks++){
                int iterate_amount_processes;
                float current_minimal_SAD = __INT_MAX__ / 2;
                tPixel_index current_best_motion_vector;
                for(iterate_amount_processes = 0; iterate_amount_processes < amount_processes - 1; iterate_amount_processes++){
                    //TODO get all lists for each macro block and compare them
                    // adjust the end_programm, so only MASTER_RANK's data gets free'd and the file_data_list from each rank
                    // TODO Seems to be working for everything except for 1 process
                    tTMP_Macro_Block_SAD buffer;
                    MPI_Recv(&buffer, 1, MPI_tMacro_Block_SAD, MPI_ANY_SOURCE, iterator_macro_blocks, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    if(buffer.value_SAD < current_minimal_SAD){
                        current_minimal_SAD = buffer.value_SAD;
                        current_best_motion_vector.x_width = buffer.x_width;
                        current_best_motion_vector.y_height = buffer.y_height;
                        xprintf(("New var: %i|%i .... buffer: %i|%i\n", current_best_motion_vector.x_width, current_best_motion_vector.y_height, buffer.x_width, buffer.y_height));
                        if(current_minimal_SAD == 0){
                            break;
                        }
                    }
                }
                tMacro_Block_SAD * tmp_new_entry = malloc(sizeof(tMacro_Block_SAD));
                tmp_new_entry->motion_vector = current_best_motion_vector;
                tmp_new_entry->value_SAD = current_minimal_SAD;

                append_element(tmp_macro_block_list, tmp_new_entry);
            }
            append_element(list_compared_pictures, tmp_macro_block_list);
        }
    } 
    if(rank != MASTER_RANK || amount_processes == 1) {
        for(i = 0; i < amount_files - 1; i++){
            int range[2];
            get_range(range, get_amount_motion_vectors(distanze_motion_vector_search));
            char * file_name = ((tFile_data *) get_element(file_data_list, i + 1)->item)->file_name;
            time_printf(("Calculating motionvectors for number %i; picture-name: %s\n", (i + 1), file_name));
            //Get all motion vectors data from one rank
            tList * tmp_list = calc_SAD_values(
                (tFile_data *) get_element(file_data_list, 0)->item,
                (tFile_data *) get_element(file_data_list, i + 1)->item,
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
                printf("tmp old: %i|%i ... buffer new: %i|%i\n", tmp->motion_vector.x_width, tmp->motion_vector.y_height, buffer.x_width, buffer.y_height);
                
                MPI_Send(&buffer, 1, MPI_tMacro_Block_SAD, MASTER_RANK, iterator_macro_block, MPI_COMM_WORLD);
            }
            // delete_list(tmp_list);
        }
    }

    gettimeofday(&calc_end_time, NULL);

#ifdef TEST_SAD_CALC_OUTPUT
    if(rank == MASTER_RANK){
        int tmp_i;
        for(tmp_i = 0; tmp_i < amount_files - 1; tmp_i++){
            xprintf(("SAD values of vectors for macro blocks between %s and %s:\n", 
                ((tFile_data *) get_element(file_data_list, 0)->item)->file_name, 
                ((tFile_data *) get_element(file_data_list, tmp_i + 1)->item)->file_name 
            ));
            int j;
            tList * tmp_output_list = (tList *) get_element(list_compared_pictures, tmp_i)->item;
            for(j = 0; j < tmp_output_list->size; j++){
                tMacro_Block_SAD * tmp_macro = (tMacro_Block_SAD *) get_element(tmp_output_list, j)->item;
                xprintf(("Block: %i; Vector: %i|%i; SAD-value: %f\n", j, tmp_macro->motion_vector.x_width, tmp_macro->motion_vector.y_height, tmp_macro->value_SAD));
                
            }
        }
    }
#endif

    add_to_evaluation_list("Calculating Motion Vectors", calc_start_time, calc_end_time, -1.0);
    time_printf(("Finished calculating the motion vectors\n"));

#ifdef TEST_ACCESS
    tPixel_data tmp;
    tmp = access_file_data_array(((tFile_data *) get_element(file_data_list, 0)->item), 0, 0);
    xprintf(("Returned: %i, %i, %i\n", tmp.red, tmp.green, tmp.blue));
    tmp = access_file_data_array(((tFile_data *) get_element(file_data_list, 0)->item), 1, 0);
    xprintf(("Returned: %i, %i, %i\n", tmp.red, tmp.green, tmp.blue));
    tmp = access_file_data_array(((tFile_data *) get_element(file_data_list, 0)->item), 0, 1);
    xprintf(("Returned: %i, %i, %i\n", tmp.red, tmp.green, tmp.blue));
    tmp = access_file_data_array(((tFile_data *) get_element(file_data_list, 0)->item), 47, 0);
    xprintf(("Returned: %i, %i, %i\n", tmp.red, tmp.green, tmp.blue));
    tmp = access_file_data_array(((tFile_data *) get_element(file_data_list, 0)->item), 47, 48);
    xprintf(("Returned: %i, %i, %i\n", tmp.red, tmp.green, tmp.blue));
    tmp = access_file_data_array(((tFile_data *) get_element(file_data_list, 0)->item), 25, 25);
    xprintf(("Returned: %i, %i, %i\n", tmp.red, tmp.green, tmp.blue));
#endif

#ifdef TEST_MACRO_CALC
    for(i = 0; i < amount_files; i++){
        tFile_data * tmp = (tFile_data *) get_element(file_data_list, i)->item;
        int amount_macro_blocks = get_amount_macro_blocks(tmp);
        xprintf(("Amount macro blocks %s: %i\n", tmp->file_name, amount_macro_blocks));

        int j;
        for(j = 0; j < amount_macro_blocks; j++){
            int index[2];
            get_macro_block_begin(tmp, j, index);
            xprintf(("Begin macro block %i: width = %i, height = %i\n", j, index[0], index[1]));
        }
    }
#endif

    time_printf(("Starting to encode all files\n"));
    struct timeval encode_start_time, encode_end_time;
    gettimeofday(&encode_start_time, NULL);

    int ret_value = encode_files(file_data_list, list_compared_pictures);
    if(ret_value == EXIT_FAILURE){
        end_programm(file_data_list, list_compared_pictures);
        
        for(i = 0; i < time_evaluation_list->size; i++){
            tTime_evaluation * tmp = (tTime_evaluation *) get_element(time_evaluation_list, i)->item;
            time_printf(("Time used for %-30s: %0.3f ms (= %0.3f s)\n", tmp->evaluation_for, tmp->time_difference, (tmp->time_difference / 1000) ));
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

    //end_programm(file_data_list, list_compared_pictures); //TODO: Remove comment later and fix end_programm

    gettimeofday(&ending_end_time, NULL);
    add_to_evaluation_list("Ending Program", ending_start_time, ending_end_time, -1.0);
    time_printf(("Finished freeing all malloced data\n"));

    MPI_Finalize();

    time_printf(("Finished running the program!\n\n"));
    
    gettimeofday(&total_end_time, NULL);

    double tmp_time_difference = 0.0;
    for(i = 0; i < time_evaluation_list->size; i++){
        tmp_time_difference += ((tTime_evaluation *) get_element(time_evaluation_list, i)->item)->time_difference;
    }
    tmp_time_difference = calculate_time_difference(total_start_time, total_end_time) - tmp_time_difference;
    
    add_to_evaluation_list("Other", total_start_time, total_end_time, tmp_time_difference);
    
    add_to_evaluation_list("Total Program", total_start_time, total_end_time, -1.0);

    time_printf(("Evaluating used time\n"));

    for(i = 0; i < time_evaluation_list->size; i++){
        tTime_evaluation * tmp = (tTime_evaluation *) get_element(time_evaluation_list, i)->item;
        time_printf(("Time used for %-30s: %0.3f ms (= %0.3f s)\n", tmp->evaluation_for, tmp->time_difference, (tmp->time_difference / 1000) ));
    }
    delete_list(time_evaluation_list);

    exit(EXIT_SUCCESS);
}