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

    time_evaluation_list = create_list();


    if(argc <= 3){
        time_printf(("Not enough args! Usage: %s <distanze_motion_vector_search> <ref_picture> <picture 1> (optional: more picturesult)\n", argv[0]));
        exit(EXIT_FAILURE);
    }

    int i;
    int amount_files = argc - 2;
    int distanze_motion_vector_search = atoi(argv[1]);

    if(distanze_motion_vector_search < 0){
        time_printf(("Given distance for the motion vector search %i is not >= 0. Please provide a value greater or equal zero\n", distanze_motion_vector_search));
        exit(EXIT_FAILURE);
    }

    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    xprintf(("I am %d of $d\n", rank, size));
    xprintf(("amount_files: %i\n\n", amount_files));
    tList * file_data_list = create_list();

    //Get the file data and store it in an array
    time_printf(("Starting to read %i files\n", amount_files));
    struct timeval read_start_time, read_end_time;
    gettimeofday(&read_start_time, NULL);
    for (i = 0; i < amount_files; i++) {
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
    for(i = 0; i < amount_files - 1; i++){
        char * file_name = ((tFile_data *) get_element(file_data_list, i + 1)->item)->file_name;
        time_printf(("Calculating motionvectors for number %i; picture-name: %s\n", (i + 1), file_name));
        append_element(
            list_compared_pictures, 
            calc_SAD_values(
                (tFile_data *) get_element(file_data_list, 0)->item,
                (tFile_data *) get_element(file_data_list, i + 1)->item,
                distanze_motion_vector_search
            )
        );
#ifdef TEST_SAD_CALC_OUTPUT
        xprintf(("SAD values of vectors for macro blocks between %s and %s:\n", 
            ((tFile_data *) get_element(file_data_list, 0)->item)->file_name, 
            ((tFile_data *) get_element(file_data_list, i + 1)->item)->file_name 
        ));
        int j;
        tList * tmp_output_list = (tList *) get_element(list_compared_pictures, i)->item;
        for(j = 0; j < tmp_output_list->size; j++){
            tMacro_Block_SAD * tmp_macro = (tMacro_Block_SAD *) get_element(tmp_output_list, j)->item;
            xprintf(("Block: %i; Vector: %i|%i; SAD-value: %f\n", j, tmp_macro->motion_vector.x_width, tmp_macro->motion_vector.y_height, tmp_macro->value_SAD));
            
        }
#endif
    }

    gettimeofday(&calc_end_time, NULL);
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
        exit(EXIT_FAILURE);
    }
    
    gettimeofday(&encode_end_time, NULL);
    add_to_evaluation_list("Encoding Files", encode_start_time, encode_end_time, -1.0);

    time_printf(("Finished encoding all files\n"));

    time_printf(("Starting to free all malloced data\n"));
    struct timeval ending_start_time, ending_end_time;
    gettimeofday(&ending_start_time, NULL);

    end_programm(file_data_list, list_compared_pictures);

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
    
    tTime_evaluation * tmp_other_evaluation = malloc(sizeof(tTime_evaluation));
    
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