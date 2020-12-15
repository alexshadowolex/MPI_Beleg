#include "list.h"

typedef struct sFile_data{
    char * file_name;
    char * data;
    int height;
    int width;
}tFile_data;

typedef struct sPixel_data{
    char red;
    char green;
    char blue;
    int initialized_correct;
}tPixel_data;

typedef struct sPixel_index{
    int x_width;
    int y_height;
}tPixel_index;

typedef struct sMacro_Block_SAD{
    float value_SAD;
    tPixel_index motion_vector;
}tMacro_Block_SAD;

void print_timestamp(void);

tFile_data * read_picture(char * file_name);
tPixel_data access_file_data_array(tFile_data * file, int width, int height);
int get_amount_macro_blocks(tFile_data * ref_picture);

void get_macro_block_begin(tFile_data * ref_picture, int number_macro_block, int index[]);
tPixel_index get_next_motion_vector(int iteration);
tList * calc_SAD_values(tFile_data * ref_picture, tFile_data * other_picture, int distanze_motion_vector_search);

int encode_files(tList * file_data, tList * compared_pictures);

#define SIZE_MACRO_BLOCK 16

#define time_printf(x) print_timestamp(); printf x

#ifdef DEBUG
#define xprintf(x) time_printf(x)
#else
#define xprintf(x)
#endif
