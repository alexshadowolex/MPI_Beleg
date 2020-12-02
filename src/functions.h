#include "list.h"

typedef struct sFile_data{
    char * file_name;
    char * data;
    int height;
    int width;
}tFile_data;

typedef struct sPixel_data{
    int red;
    int green;
    int blue;
}tPixel_data;

typedef struct sMotion_Vector{
    int x_width;
    int y_height;
}tMotion_Vector;

typedef struct sMacro_Block_SAD{
    float value_SAD;
    tMotion_Vector motion_vector;
}tMacro_Block_SAD;

void print_timestamp(void);

tFile_data * read_picture(char * file_name);
tPixel_data access_file_data_array(tFile_data * file, int width, int height);
int get_amount_macro_blocks(tFile_data * ref_picture);
void get_macro_block_begin(tFile_data * ref_picture, int number_macro_block, int index[]);
tList * calc_SAD_values(tFile_data * ref_picture, tFile_data * other_picture, int distanze_motion_vector_search);

#ifdef DEBUG
#define xprintf(x) printf x
#else
#define xprintf(x)
#endif