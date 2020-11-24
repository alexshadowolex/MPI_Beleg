#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "functions.h"

int main(int argc, char ** argv){
    // float hdr_data[200][200][3];

    if(argc <= 2){
        printf("Not enough args! Usage: %s <ref_picture> <picture 1> (optional: more picturesult)\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int i;
    int amount_files = argc - 1;
    xprintf(("amount_files: %i\n", amount_files));
    tFile_data * file_data[amount_files];

    //Get the file data and store it in an array
    for (i = 0; i < amount_files; i++) {
        char * tmp_file_name = argv[i + 1];
        tFile_data * tmp_data = read_picture(tmp_file_name);
        printf("Reading file %s finished!\n", tmp_data->file_name);
        xprintf(("Data_size: %i | Picture_width: %i | Picture_Height: %i\n", (sizeof(tmp_data->data)), tmp_data->width, tmp_data->height));
        // int j,
        // for(j = 0; j < )
        file_data[i] = tmp_data;
    }

    float values_SAD[amount_files - 1];

    for(i = 0; i < amount_files - 1; i++){
        values_SAD[i] = calculate_SAD(file_data[0], file_data[i + 1]);
        xprintf(("SAD-value between %s and %s: %f\n", file_data[0]->file_name, file_data[i + 1]->file_name, values_SAD[i]));
    }
    exit(EXIT_SUCCESS);
}