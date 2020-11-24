
typedef struct sFile_data{
    char * file_name;
    char * data;
    int height;
    int width;
}tFile_data;

tFile_data * read_picture(char * file_name);

float calculate_SAD(tFile_data * data_ref_picture, tFile_data * data_other_picture);

#ifdef DEBUG
#define xprintf(x) printf x
#else
#define xprintf(x)
#endif