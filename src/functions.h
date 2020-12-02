
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

tFile_data * read_picture(char * file_name);
tPixel_data access_file_data_array(tFile_data * file, int width, int height);
float calculate_SAD(tFile_data * data_ref_picture, tFile_data * data_other_picture);
int get_amount_macro_blocks(tFile_data * ref_picture);

#ifdef DEBUG
#define xprintf(x) printf x
#else
#define xprintf(x)
#endif