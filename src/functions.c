#include <time.h>
#include <stdio.h>
#include <sys/time.h>
#include "functions.h"
#if __has_include(<mpi.h>)
#   include <mpi.h>
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb/stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb/stb_image.h"

#define STB_DEFINE
#include "../lib/stb/stb.h"

// #define PNGSUITE_PRIMARY
#define TERM_OUTPUT


// #define X11_DISPLAY


// Globally used vars


// Print a timestamp for Output
void print_timestamp(void){
    char buffer[26];
    int millisec;
    struct tm * tm_info;
    struct timeval tv;

    gettimeofday(&tv, NULL);

    millisec = lrint(tv.tv_usec/1000.0); // Round to nearest millisec
    if (millisec >= 1000) { // Allow for rounding up to nearest second
        millisec -=1000;
        tv.tv_sec++;
    }

    tm_info = localtime(&tv.tv_sec);


    strftime(buffer, 26, "%H:%M:%S", tm_info);
    printf("%s.%03d ", buffer, millisec);
}

// Initialize datatypes used for mpi_send
void init_mpi_data_types(void){
    int macro_block_SAD_lenghts[3] = {1, 1, 1};
    MPI_Aint macro_block_SAD_displacements[3] = {0, sizeof(float), sizeof(float) + sizeof(int)};
    MPI_Datatype macro_block_SAD_types[3] = {MPI_FLOAT, MPI_INT, MPI_INT};
    
    MPI_Type_create_struct(3, macro_block_SAD_lenghts, macro_block_SAD_displacements, macro_block_SAD_types, &MPI_tMacro_Block_SAD);
    MPI_Type_commit(&MPI_tMacro_Block_SAD);
}

// ===================Reader Functions===================
// Read the data of a jpg-File and return it as a tFile_data-struct
tFile_data * read_picture(char * file_name){
    int result;
    int width, height;
    int w2,h2,n2;
    int n;
    unsigned char * data;
    tFile_data * tmp;
    result = stbi_info(file_name, &w2, &h2, &n2);

    data = stbi_load(file_name, &width, &height, &n, 4); 
    if (data){ 
        free(data);
    } else {
        time_printf(("Failed loading data of picture %s\n", file_name));
    }

    // load image once again and check whether the same values are obtained 
    data = stbi_load(file_name, &width, &height, &n, 4);
    assert(data);
    assert(width == w2 && height == h2 && n == n2);
    assert(result);

    if (data) {
        // Store data in struct
        tmp = malloc(sizeof(tFile_data));
        tmp->file_name = malloc(strlen(file_name) + 1);
        strcpy(tmp->file_name, file_name); 
        tmp->data = malloc(height * width * 4);
        memcpy(tmp->data, data, height * width * 4);
        tmp->height = height; 
        tmp->width = width;
    } else {
        time_printf(("Failed loading data on second try, picture %s\n", file_name));
    }
    return tmp;
}

// Simulate 2D-access onto the data-array and transform it into 1D-access
tPixel_data access_file_data_array(tFile_data * file, int x_width, int y_height){
    int access_index = y_height * file->width * 4 + x_width * 4;
#ifdef TEST_ACCESS
    xprintf(("access_index = %i\n", access_index));
#endif
    tPixel_data ret_value = {
        '\0',
        '\0',
        '\0'
    };
    // if the index is out of bound, set the initialized_correct to false
    if(y_height >= file->height || x_width >= file->width || y_height < 0 || x_width < 0){
        ret_value.initialized_correct = 0;
    } else {
        ret_value.red = (unsigned char) file->data[access_index + 0];
        ret_value.green = (unsigned char) file->data[access_index + 1];
        ret_value.blue = (unsigned char) file->data[access_index + 2];
        ret_value.initialized_correct = 1;
    }
    return ret_value;
}

// Return the amount of macro blocks in a picture
int get_amount_macro_blocks(tFile_data * ref_picture){
    // Since every picture has an integer amount of macro blocks, the calculation is easy
    return (ref_picture->height / SIZE_MACRO_BLOCK) * (ref_picture->width / SIZE_MACRO_BLOCK);
}

// ===================SAD Functions===================
/* Macro blocks are counted like this:
 * 0-1-2
 * 3-4-5
 * 6-7-8
 * The function returns the index of the left-upper pixel of a macro block (first: 0,0; second: 0,16;...)
 */
void get_macro_block_begin(tFile_data * ref_picture, int number_macro_block, int index[]){
    // returns: int[]{width, height}, pointing at the index of the left upper pixel
    if(number_macro_block >= get_amount_macro_blocks(ref_picture)){
        return;
    }
    int blocks_per_line = ref_picture->width / SIZE_MACRO_BLOCK;
    int line = number_macro_block / blocks_per_line;
    int col = number_macro_block % blocks_per_line;

    int pixel_width = col * SIZE_MACRO_BLOCK;
    int pixel_height = line * SIZE_MACRO_BLOCK;

    index[0] = pixel_width;
    index[1] = pixel_height;
}

// Calculate the given distance for motion vectors into the amount of all motion vectors inside the distance
int get_amount_motion_vectors(int distance_motion_vector){
    // for the distance 1, the amount is 9, for 2 it's 25 etc.
    if(distance_motion_vector < 0){
        return -1;
    }
    return ((distance_motion_vector * 2) + 1) * ((distance_motion_vector * 2) + 1);
}

// Calculate the range of motion vectors to check for each rank
void get_range(int range[], int amount_motion_vectors){
    int amount_working_processes = amount_processes - 1;
    if(amount_working_processes == 0){
        range[0] = 0;
        range[1] = amount_motion_vectors;
    } else {
        range[0] = (amount_motion_vectors / amount_working_processes) * (rank - 1);
        if(rank == amount_working_processes){
            range[1] = amount_motion_vectors;
        } else {
            range[1] = (amount_motion_vectors / amount_working_processes) * (rank);
        }
        // Caclulate the rest of motion vectors, which are all left for the last rank
        int rest = amount_motion_vectors % amount_working_processes;
        int working_rank = rank - 1;
        if(working_rank != amount_working_processes - 1 && rest > 0){
            int move_end = 0;
            if(rank <= rest){
                move_end = working_rank + 1;
            } else {
                move_end = rest;
            }
            range[1] += move_end; 
        }
        if(working_rank != 0 && rest > 0){
            int move_begin = 0;
            if(rank <= rest){
                move_begin = working_rank;
            } else {
                move_begin = rest;
            }
            range[0] += move_begin; 
        }
    }
}

// Gets next motion vector as snail like iteration through all possibilities (e.g. iteration_best_motion_vector.png)
tPixel_index get_next_motion_vector(int iteration){
    int tmp_x;
    int tmp_y;

    // Checking, which distance we are searching through with the current iteration
    int current_distance = 0;
    while(1){
        if(((current_distance * 2) + 1) * ((current_distance * 2) + 1) > iteration){
            break;
        }
        current_distance++;
    }

    if(current_distance == 0){
        tmp_x = 0;
        tmp_y = 0;
    } else {
        // Substract all possible motion vectors that happened before our current distance
        iteration -= get_amount_motion_vectors(current_distance - 1);
        int tmp_iteration;
        int move_x;
        int move_y;
        for(tmp_iteration = 0; tmp_iteration <= iteration; tmp_iteration++){
            if(tmp_iteration == 0){ 
                tmp_y = current_distance;
                tmp_x = 0;
                move_x = 1;
                move_y = 0;
            } else {
                // Checking if we reached one corner, so we can change the direction we are moving towards
                if(move_x != 0 && abs(tmp_x) == current_distance){
                    // Corner reached and x was moving, stop moving x
                    move_x = 0;
                    // Check where y needs to get move towards
                    // If y is at the postive end, move to towards the negative end
                    if(tmp_y == current_distance){
                        move_y = -1;
                    } else {
                        move_y = 1;
                    }
                } else {
                    // Only one can happen in a direction change
                    if(move_y != 0 && abs(tmp_y) == current_distance){
                        move_y = 0;
                        if(tmp_x == current_distance){
                            move_x = -1;
                        } else {
                            move_x = 1;
                        }
                    }
                }
                
                tmp_x += move_x;
                tmp_y += move_y;
            }
        }
    }

    tPixel_index ret_vector;
    ret_vector.x_width = tmp_x;
    ret_vector.y_height = tmp_y;
    return ret_vector;
}

// Calculates the SAD values for all possible motion vectors for all macro blocks
tList * calc_SAD_values(tFile_data * ref_picture, tFile_data * other_picture, int distanze_motion_vector_search, int range_start, int range_end){
    // The list holds for all macro blocks the best motion vector in a struct tMacro_Block_SAD
    tList * all_macro_block_SAD = create_list();
    int amount_macro_blocks = get_amount_macro_blocks(ref_picture);
    int current_macro_block, current_x_width_motion, current_y_height_motion, x_current_width_macro_block, y_current_height_macro_block;

    int amount_motion_vectors = get_amount_motion_vectors(distanze_motion_vector_search);
#ifdef TEST_SAD_CALC
    xprintf(("Distance motion vector: %i\n", distanze_motion_vector_search));
    xprintf(("Amount motion vectors: %i\n", amount_motion_vectors));
#endif
    for(current_macro_block = 0; 
        current_macro_block < amount_macro_blocks; 
        current_macro_block++){

        // Compare all macro blocks
        int begin_index[2];
        get_macro_block_begin(ref_picture, current_macro_block, begin_index);
        int x_width_motion = INT_MAX;
        int y_height_motion = INT_MAX;
        float minimal_SAD = INT_MAX;

        int current_motion_vector_iteration;
        int found_minimal_SAD = 0;

        int rank_has_best_SAD_value = 1;
        int size_alltoall_buffer = amount_processes == 1 ? 1 : amount_processes - 1;
        tTMP_Macro_Block_SAD alltoall_buffer[size_alltoall_buffer];
        // get_next_motion_vector will return values for the iteration
        // amount_motion_vectors is the amount of motion vectors that have to get tested
        for(current_motion_vector_iteration = range_start; 
            current_motion_vector_iteration < range_end && !found_minimal_SAD; 
            current_motion_vector_iteration++){

            tPixel_index next_motion_vector = get_next_motion_vector(current_motion_vector_iteration);
            tTMP_Macro_Block_SAD current_values;
            current_values.x_width = next_motion_vector.x_width;
            current_values.y_height = next_motion_vector.y_height;
            current_values.value_SAD = 0;
            int exceeded_minimal_sad = 0;
            // Calculate minimal SAD and save the value and the fitting distance motion vector
            // Iterate over all pixels in a macro block (SIZE_MACRO_BLOCK x SIZE_MACRO_BLOCK)
            for(x_current_width_macro_block = begin_index[0]; 
                x_current_width_macro_block < begin_index[0] + SIZE_MACRO_BLOCK && !exceeded_minimal_sad;
                x_current_width_macro_block++){
                for(y_current_height_macro_block = begin_index[1]; 
                    y_current_height_macro_block < begin_index[1] + SIZE_MACRO_BLOCK && !exceeded_minimal_sad; 
                    y_current_height_macro_block++){
                    
                    // Get current pixeldata from ref_picture
                    tPixel_data ref_pixel = access_file_data_array(ref_picture, x_current_width_macro_block + current_values.x_width, y_current_height_macro_block + current_values.y_height);
                    // Get current pixeldata from other_picture, moved by current motion vector
                    tPixel_data other_pixel = access_file_data_array(other_picture, x_current_width_macro_block, y_current_height_macro_block);

                    if(!ref_pixel.initialized_correct){
                        // Means we tried to access a pixel outside of the picture
                        current_values.value_SAD += INT_MAX / 2;
                        continue;
                    }
                    unsigned char ref_brightness = (unsigned char) ((30 * ref_pixel.red + 59 * ref_pixel.green + 11 * ref_pixel.blue) / 100);
                    unsigned char other_brightness = (unsigned char) ((30 * other_pixel.red + 59 * other_pixel.green + 11 * other_pixel.blue) / 100);
                    unsigned char value;
                    // Calculate the difference. Since there is no abs-function for unsigned chars, we need to check the case before
                    if(ref_brightness >= other_brightness){
                        value = ref_brightness - other_brightness;
                    } else {
                        value = other_brightness - ref_brightness;
                    }
                    // Add to the current_SAD-Value. If it already exceeded the mininmal SAD value, we can stop checking this motion vector
                    current_values.value_SAD += (int) value;
                    MPI_Request request;
                    MPI_Ialltoall(&current_values, 1, MPI_tMacro_Block_SAD, &alltoall_buffer, 1, MPI_tMacro_Block_SAD, MPI_COMM_WORLD, &request);
                    int alltoall_iterator;
                    for(alltoall_iterator = 0; alltoall_iterator < size_alltoall_buffer; alltoall_iterator++){
                        printf("%i current_macro_block: Current SAD: %f, Current X: %i, Current y: %i, Alltoall SAD: %f\n",current_macro_block, current_values.value_SAD, current_values.x_width, current_values.y_height, alltoall_buffer[alltoall_iterator].value_SAD);
                        if(alltoall_buffer[alltoall_iterator].value_SAD < current_values.value_SAD){
                            exceeded_minimal_sad = 1;
                            break;
                        }
                    }
                }
            }
            // Comparing minimal sad with the current vector and minimal SAD
            if(current_values.value_SAD == 0){
                // Will stop the iterations, 0 is the total minimum
                found_minimal_SAD = 1;
            }

            MPI_Alltoall(&current_values, 1, MPI_tMacro_Block_SAD, &alltoall_buffer, 1, MPI_tMacro_Block_SAD, MPI_COMM_WORLD);

            int alltoall_iterator;
            for(alltoall_iterator = 0; alltoall_iterator < size_alltoall_buffer; alltoall_iterator++){
                if(current_values.value_SAD > alltoall_buffer[alltoall_iterator].value_SAD){
                    rank_has_best_SAD_value = 0;
                }
                if(current_values.value_SAD == alltoall_buffer[alltoall_iterator].value_SAD && current_values.value_SAD == 0){
                    if(current_values.x_width != 0 || current_values.y_height != 0){
                        rank_has_best_SAD_value = 0;
                    } else {
                        rank_has_best_SAD_value = 1;
                    }
                }
            }
            if(rank_has_best_SAD_value){
                minimal_SAD = current_values.value_SAD;
                x_width_motion = current_values.x_width;
                y_height_motion = current_values.y_height;
            }

            // Save the lowest sad Value and the fitting motion vector
#ifdef TEST_SAD_CALC
                xprintf(("Found new minimal SAD: %f\n", current_values.value_SAD));
#endif
        }
#ifdef TEST_SAD_CALC
        printf("Current macro block: %i\nMotion Vector: x_width = %i, y_height = %i\nSAD-value: %f\n", current_macro_block, x_width_motion, y_height_motion, minimal_SAD);
#endif
        // Add vector for macro block here
        tMacro_Block_SAD * macro_block_SAD = malloc(sizeof(tMacro_Block_SAD));
        if(rank_has_best_SAD_value){
            tPixel_index motion_vector = {x_width_motion, y_height_motion};
            
            macro_block_SAD->value_SAD = minimal_SAD;
            macro_block_SAD->motion_vector = motion_vector;
            printf("Vector: %i-%i, Value: %f\n", motion_vector.x_width, motion_vector.y_height, minimal_SAD);
        } else {
            macro_block_SAD = NULL;
        }
        append_element(all_macro_block_SAD, macro_block_SAD);
    }

    return all_macro_block_SAD;
}

// ===================En-/Decode Functions===================
// Encode the files into a <file_name>.bpg binary file
int encode_files(tList * file_data, tList * compared_pictures){
    // All data will be seperated by a single space character
    int i;
    // Get the reference picture and data
    tFile_data * ref_picture = (tFile_data *) get_element(file_data, 0)->item;

    for(i = 0; i < compared_pictures->size; i++){
        // Get the current picture to encode
        tFile_data * current_picture = (tFile_data *) get_element(file_data, i + 1)->item;

        char file_name[strlen(current_picture->file_name) + 4];
        strcpy(file_name, current_picture->file_name);
        strcat(file_name, ".bpg");
        xprintf(("file_name: %s\n", file_name));
        FILE * file = fopen(file_name, "w+");

        // Write the amount of macroblocks to the file
        int amount_macro_blocks = get_amount_macro_blocks(current_picture);
        fwrite(&amount_macro_blocks, sizeof(int), 1, file);

        // Get all macro blocks data
        tList * current_all_macro_blocks = (tList *) get_element(compared_pictures, i)->item;
        // Write all motion vectors to the file
        int iterator_motion_vectors;
        for(iterator_motion_vectors = 0; iterator_motion_vectors < current_all_macro_blocks->size; iterator_motion_vectors++){
            tMacro_Block_SAD * tmp_block_info = (tMacro_Block_SAD *) get_element(current_all_macro_blocks, iterator_motion_vectors)->item;
            tPixel_index current_motion_vector = tmp_block_info->motion_vector;

            fwrite(&current_motion_vector.x_width, sizeof(int), 1, file);
            fwrite(&current_motion_vector.y_height, sizeof(int), 1, file);
        }
        xprintf(("Wrote motion vectors\n"));

        // Write all decoded data
        int iterator_macro_blocks;
        for(iterator_macro_blocks = 0; iterator_macro_blocks < current_all_macro_blocks->size; iterator_macro_blocks++){
            tMacro_Block_SAD * tmp_block_info = (tMacro_Block_SAD *) get_element(current_all_macro_blocks, iterator_macro_blocks)->item;
            tPixel_index motion_vector = tmp_block_info->motion_vector;
            int begin_macro_block[2];
            get_macro_block_begin(current_picture, iterator_macro_blocks, begin_macro_block);
            int width, height;

            for(width = begin_macro_block[0]; width < begin_macro_block[0] + SIZE_MACRO_BLOCK; width++){
                for(height = begin_macro_block[1]; height < begin_macro_block[1] + SIZE_MACRO_BLOCK; height++){
                    tPixel_data tmp_current_data = access_file_data_array(current_picture, width, height);
                    tPixel_data tmp_ref_data = access_file_data_array(ref_picture, width + motion_vector.x_width, height + motion_vector.y_height);
                    
                    // Getting the difference as a signed short
                    // Difference is always ref_pixeld_data - encode_pixel_data
                    signed short red_short = (signed short) (tmp_ref_data.red - tmp_current_data.red);
                    signed short green_short = (signed short) (tmp_ref_data.green - tmp_current_data.green);
                    signed short blue_short = (signed short) (tmp_ref_data.blue - tmp_current_data.blue);

                    tEncode_pixel_data * resulting_data = malloc(sizeof(tEncode_pixel_data));
                    resulting_data->red = red_short;
                    resulting_data->green = green_short;
                    resulting_data->blue = blue_short;
                    resulting_data->dummy = 0;

                    fwrite(resulting_data, sizeof(tEncode_pixel_data), 1, file);
                    
                    free(resulting_data);
                }
            }
        }
    }

    return EXIT_SUCCESS;
}

// ===================Evaluation Functions===================
// Calculate the time difference 
double calculate_time_difference(struct timeval start_time, struct timeval end_time){
    return ((end_time.tv_sec - start_time.tv_sec) * 1000000 + end_time.tv_usec - start_time.tv_usec) / 1000.0;
}

// Calculate and add a value to the evaluation list
void add_to_evaluation_list(char * evaluation_for, struct timeval start_time, struct timeval end_time, double calculated_difference){
    tTime_evaluation * tmp_evaluation = malloc(sizeof(tTime_evaluation));

    // If there is no real distance given, calculate it 
    if(calculated_difference == -1.0){
        tmp_evaluation->time_difference = calculate_time_difference(start_time, end_time);
    } else {
        tmp_evaluation->time_difference = calculated_difference;
    }

    tmp_evaluation->evaluation_for = malloc(strlen(evaluation_for) + 1);
    strcpy(tmp_evaluation->evaluation_for, evaluation_for);
    append_element(time_evaluation_list, tmp_evaluation);
}

// ===================End Programm Functions===================
// Function to free all malloc'ed data
void end_programm(tList * file_data_list, tList * list_compared_pictures){
    int i;

    // Delete all file datas
    tList_Element * current_element = file_data_list->first_element;
    for(i = 0; i < file_data_list->size; i++){
        // Delete the tFile_data->data first
        free(((tFile_data *) current_element->item)->data);
        free(((tFile_data *) current_element->item)->file_name);
        if(current_element->next_element != NULL){
            current_element = current_element->next_element;
        }
    }
    // Now delete the whole list
    delete_list(file_data_list);

    // Delete all macro block data of compared pictures
    current_element = list_compared_pictures->first_element;
    for(i = 0; i < list_compared_pictures->size; i++){
        delete_list((tList *) current_element->item);
        if(current_element->next_element != NULL){
            current_element = current_element->next_element;
        }
    }

    // Now delete the whole list (again). Using the function would free the (already freed) items again, which causes leaks
    tList_Element * tmp_next_element = list_compared_pictures->first_element;
    do{
        tList_Element * tmp = tmp_next_element;
        tmp_next_element = tmp->next_element;
        free(tmp);
    }while(tmp_next_element != NULL);
    free(list_compared_pictures);
}