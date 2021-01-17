#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "functions.h"

int main(int argc, char ** argv){

    if(argc <= 3){
        time_printf(("Not enough args! Usage: %s <distanze_motion_vector_search> <ref_picture> <picture 1> (optional: more pictures)\n", argv[0]));
        exit(EXIT_FAILURE);
    }

    int i;
    int amount_files = argc - 2;
    int distanze_motion_vector_search = atoi(argv[1]);

    if(distanze_motion_vector_search < 0){
        time_printf(("Given distance for the motion vector search %i is not >= 0. Please provide a value greater or equal zero\n", distanze_motion_vector_search));
        exit(EXIT_FAILURE);
    }
    xprintf(("amount_files: %i\n\n", amount_files));
    tList * file_data_list = create_list();

    //Get the file data and store it in an array
    time_printf(("Starting to read %i files\n", amount_files));
    for (i = 0; i < amount_files; i++) {
        char * tmp_file_name = argv[i + 2];
        tFile_data * tmp_data = read_picture(tmp_file_name);
        time_printf(("Reading file %s finished!\n", tmp_data->file_name));
        xprintf(("Data_size: %i | Picture_width: %i | Picture_Height: %i\n\n", (sizeof(tmp_data->data)), tmp_data->width, tmp_data->height));
        append_element(file_data_list, tmp_data);
    }

    //This list is holding lists of tMacro_Block_SAD. At index 0, file_data_list[0] and file_data_list[1] are compared, 
    //at index 1 file_data_list[0] and file_data_list[2] are compared etc.
    tList * list_compared_pictures = create_list();

    time_printf(("Starting to calculate the motion vectors\n"));
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

    int ret_value = encode_files(file_data_list, list_compared_pictures);
    if(ret_value == EXIT_FAILURE){
        exit(EXIT_FAILURE);
    }

    time_printf(("Finished encoding all files\n"));

    time_printf(("Starting to free all maloced data\n"));

    end_programm(file_data_list, list_compared_pictures);

    time_printf(("Finished freeing all maloced data\n"));

    time_printf(("Time used for Other : 3.007 ms (=0.003 s)\n"));

    time_printf(("Finished running the program!\n"));

    time_printf(("Time used for Programm : 1.667 ms (=0.001 s)\n"));

    exit(EXIT_SUCCESS);
}