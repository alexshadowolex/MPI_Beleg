#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "functions.h"
#include "list.h"

int main(int argc, char ** argv){
    // float hdr_data[200][200][3];

    if(argc <= 3){
        printf("Not enough args! Usage: %s <distanze_motion_vector_seatch> <ref_picture> <picture 1> (optional: more picturesult)\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int i;
    int amount_files = argc - 2;
    int distanze_motion_vector_seatch = atoi(argv[1]);
    xprintf(("amount_files: %i\n", amount_files));
    tList * file_data_list = create_list();

    //Get the file data and store it in an array
    for (i = 0; i < amount_files; i++) {
        char * tmp_file_name = argv[i + 2];
        tFile_data * tmp_data = read_picture(tmp_file_name);
        printf("Reading file %s finished!\n", tmp_data->file_name);
        xprintf(("Data_size: %i | Picture_width: %i | Picture_Height: %i\n", (sizeof(tmp_data->data)), tmp_data->width, tmp_data->height));
        append_element(file_data_list, tmp_data);
    }

    tList * SAD_values_list = create_list();

    for(i = 0; i < amount_files - 1; i++){
        float * tmp_SAD = malloc(sizeof(float));
        *tmp_SAD = calculate_SAD( (tFile_data *) get_element(file_data_list, 0)->item, (tFile_data *) get_element(file_data_list, i + 1)->item);
        // xprintf(("SAD VALUE: %f\n", *tmp_SAD));
        append_element(SAD_values_list, (float *)tmp_SAD);
        xprintf(("SAD-value between %s and %s: %f\n", ((tFile_data *) get_element(file_data_list, 0)->item)->file_name, 
                                                      ((tFile_data *) get_element(file_data_list, i + 1)->item)->file_name, 
                                                      ((float *) get_element(SAD_values_list, i)->item)
        ));
    }
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
        tFile_data * tmp = (tFile_data *) get_element(file_data_list, 0)->item;
        xprintf(("Amount macro blocks %s: %i\n", tmp->file_name, get_amount_macro_blocks(tmp)));
    }
#endif

    exit(EXIT_SUCCESS);
}