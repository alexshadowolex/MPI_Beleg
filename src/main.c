#include <stdlib.h>
#include <stdio.h>
#include "functions.h"

int main(int argc, char ** argv){
    // float hdr_data[200][200][3];

    if(argc <= 2){
        printf("Not enough args! Usage: %s <ref_picture> <picture 1> (optional: more picturesult)\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int i;
    const int amount_files = argc;
    tFile_data file_data[amount_files];

    //Get the file data and store it in an array
    for (i = 1; i < argc; i++) {
        char * tmp_file_name = argv[i];
        file_data[i - 1] = read_picture(tmp_file_name);
    }

    long values_SAD[amount_files - 1];

    for(i = 0; i < amount_files - 1; i++){
        // values_SAD[i] = calculate_SAD(file_data[0], file_data[i + 1]);
    }
    exit(EXIT_SUCCESS);
}