#include <sys/time.h>
#include "list.h"
#if __has_include(<mpi.h>)
#   include <mpi.h>
#endif

typedef struct sFile_data{
    char * file_name;
    char * data;
    int height;
    int width;
}tFile_data;

typedef struct sPixel_data{
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    int initialized_correct;
}tPixel_data;

typedef struct sEncode_pixel_data{
    signed short red;
    signed short green;
    signed short blue;
    signed short dummy; // just so the struct size is 8 instead of 6 byte
}tEncode_pixel_data;

typedef struct sPixel_index{
    int x_width;
    int y_height;
}tPixel_index;

typedef struct sTime_evaluation{
    double time_difference;
    char * evaluation_for;
}tTime_evaluation;

typedef struct sMacro_Block_SAD{
    float value_SAD;
    tPixel_index motion_vector;
}tMacro_Block_SAD;

typedef struct sTMP_Macro_Block_SAD{
    float value_SAD;
    int x_width;
    int y_height;
}tTMP_Macro_Block_SAD;

MPI_Datatype MPI_tMacro_Block_SAD;
MPI_Comm worker;

tList * time_evaluation_list;
int rank, amount_processes;

void print_timestamp(void);
void init_mpi_data_types(void);

tFile_data * read_picture(char * file_name);
tPixel_data access_file_data_array(tFile_data * file, int width, int height);
int get_amount_macro_blocks(tFile_data * ref_picture);

void get_macro_block_begin(tFile_data * ref_picture, int number_macro_block, int index[]);
void get_range(int range[], int amount_motion_vectors);
int get_amount_motion_vectors(int distance_motion_vector);
tPixel_index get_next_motion_vector(int iteration);
tList * calc_SAD_values(tFile_data * ref_picture, tFile_data * other_picture, int distanze_motion_vector_search, int range_start, int range_end);

int encode_files(tList * file_data, tList * compared_pictures);

double calculate_time_difference(struct timeval start_time, struct timeval end_time);
void add_to_evaluation_list(char * evaluation_for, struct timeval start, struct timeval end, double calculated_difference);

void end_programm(tList * file_data_list, tList * list_compared_pictures);

#define SIZE_MACRO_BLOCK 16
#define MASTER_RANK 0

#define time_printf(x) if(rank == 0){print_timestamp(); printf x;}

#ifdef DEBUG
#define xprintf(x) time_printf(x)
#else
#define xprintf(x)
#endif
