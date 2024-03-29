#include "functions.h"
#include <time.h>
#include <stdio.h>
#include <sys/time.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb/stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb/stb_image.h"

#define STB_DEFINE
#include "../lib/stb/stb.h"

//#define PNGSUITE_PRIMARY
#define TERM_OUTPUT


//#define X11_DISPLAY


//Globally used vars


//Timestamp
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

//===================Reader Functions===================
tFile_data * read_picture(char * file_name){
    int result;
    int width, height;
    int w2,h2,n2;
    int tmp_height, tmp_width;
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
    if(data){
        tmp = malloc(sizeof(tFile_data));
        tmp->file_name = file_name;
        tmp->data = (char *) malloc((height - 1) * width * 4 + (width - 1) * 4 + 2);
        memcpy(tmp->data, data, (height - 1) * width * 4 + (width - 1) * 4 + 2);
        tmp->height = height; 
        tmp->width = width;
    } else {
        time_printf(("Failed loading data on second try, picture %s\n", file_name));
    }
    return tmp;
}

tPixel_data access_file_data_array(tFile_data * file, int x_width, int y_height){
    int access_index = y_height * file->width * 4 + x_width * 4;
#ifdef TEST_ACCESS
    xprintf(("access_index = %i\n", access_index));
#endif
    tPixel_data ret_value = {
        '0',
        '0',
        '0'
    };
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

int get_amount_macro_blocks(tFile_data * ref_picture){
    //Since every picture has an integer amount of macro blocks, the calculation is easy
    return (ref_picture->height / SIZE_MACRO_BLOCK) * (ref_picture->width / SIZE_MACRO_BLOCK);
}

//===================SAD Functions===================
/* Macro blocks are counted like this:
 * 0-1-2
 * 3-4-5
 * 6-7-8
 * The function returns the index of the left-upper pixel of a macro block (first: 0,0; second: 0,16;...)
 */
void get_macro_block_begin(tFile_data * ref_picture, int number_macro_block, int index[]){
    //returns: int[]{width, height}, pointing at the index of the left upper pixel
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

//Gets next motion vector as snail like iteration through all possibilities
tPixel_index get_next_motion_vector(int iteration){
    int tmp_x;
    int tmp_y;

    //Checking, which distance we are searching through with the current iteration
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
        //Substract all possible motion vectors that happened before our current distance
        iteration -= (((current_distance - 1) * 2) + 1) * (((current_distance - 1) * 2) + 1);
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
                    //Corner reached and x was moving, stop moving x
                    move_x = 0;
                    //Check where y needs to get move towards
                    //If y is at the postive end, move to towards the negative end
                    if(tmp_y == current_distance){
                        move_y = -1;
                    } else {
                        move_y = 1;
                    }
                } else {
                    //Only one can happen in a direction change
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

//Calculates the SAD values for all possible motion vectors for all macro blocks
tList * calc_SAD_values(tFile_data * ref_picture, tFile_data * other_picture, int distanze_motion_vector_search){
    //The list holds for all macro blocks the best motion vector in a struct tMacro_Block_SAD
    tList * all_macro_block_SAD = create_list();
    int amount_macro_blocks = get_amount_macro_blocks(ref_picture);
    int i, current_x_width_motion, current_y_height_motion, x_current_width_macro_block, y_current_height_macro_block;
    //for the distance 1, the amount is 9, for 2 it's 25 etc.
    int amount_motion_vectors = ((distanze_motion_vector_search * 2) + 1) * ((distanze_motion_vector_search * 2) + 1);
#ifdef TEST_SAD_CALC
    xprintf(("Distance motion vector: %i\n", distanze_motion_vector_search));
    xprintf(("Amount motion vectors: %i\n", amount_motion_vectors));
#endif
    for(i = 0; i < amount_macro_blocks; i++){
        //Compare all macro blocks
        int begin_index[2];
        get_macro_block_begin(ref_picture, i, begin_index);
        float minimal_SAD = INT_MAX;
        int x_width_motion;
        int y_height_motion;
        int j;
        int found_minimal_SAD = 0;
        //get_next_motion_vector will return values for the iteration
        //amount_motion_vectors is the amount of motion vectors that have to get tested
        for(j = 0; j < amount_motion_vectors && !found_minimal_SAD; j++){
            tPixel_index next_motion_vector = get_next_motion_vector(j);
            current_x_width_motion = next_motion_vector.x_width;
            current_y_height_motion = next_motion_vector.y_height;
            float current_SAD = 0;
            int exceeded_minimal_sad = 0;
            //Calculate minimal SAD and save the value and the fitting distance motion vector
            //Iterate over all pixels in a macro block (SIZE_MACRO_BLOCK x SIZE_MACRO_BLOCK)
            for(x_current_width_macro_block = begin_index[0]; 
                x_current_width_macro_block < begin_index[0] + SIZE_MACRO_BLOCK && !exceeded_minimal_sad;
                x_current_width_macro_block++){
                for(y_current_height_macro_block = begin_index[1]; 
                    y_current_height_macro_block < begin_index[1] + SIZE_MACRO_BLOCK && !exceeded_minimal_sad; 
                    y_current_height_macro_block++){
                    
                    //Get current pixeldata from ref_picture
                    tPixel_data ref_pixel = access_file_data_array(ref_picture, x_current_width_macro_block + current_x_width_motion, y_current_height_macro_block + current_y_height_motion);
                    //Get current pixeldata from other_picture, moved by current motion vector
                    tPixel_data other_pixel = access_file_data_array(other_picture, x_current_width_macro_block, y_current_height_macro_block);

                    if(!ref_pixel.initialized_correct){
                        //Means we tried to access a pixel outside of the picture
                        current_SAD += INT_MAX / 2;
                        continue;
                    }
                    unsigned char ref_brightness = (unsigned char) ((30 * ref_pixel.red + 59 * ref_pixel.green + 11 * ref_pixel.blue) / 100);
                    unsigned char other_brightness = (unsigned char) ((30 * other_pixel.red + 59 * other_pixel.green + 11 * other_pixel.blue) / 100);
                    unsigned char value;
                    if(ref_brightness >= other_brightness){
                        value = ref_brightness - other_brightness;
                    } else {
                        value = other_brightness - ref_brightness;
                    }
                    current_SAD += (int) value;
                    if(current_SAD > minimal_SAD){
                        exceeded_minimal_sad = 1;
                    }
                }
            }
            //Comparing minimal sad with the current vector and minimal SAD
            if(current_SAD == 0){
                //Will stop the iterations, 0 is the total minimum
                found_minimal_SAD = 1;
            }
            if(current_SAD < minimal_SAD){
                //Save the lowest sad Value and the fitting motion vector
#ifdef TEST_SAD_CALC
                xprintf(("Found new minimal SAD: %f\n", current_SAD));
#endif
                minimal_SAD = current_SAD;
                x_width_motion = current_x_width_motion;
                y_height_motion = current_y_height_motion;
            }
        }
        //Add vector for macro block here
#ifdef TEST_SAD_CALC
        xprintf(("Current macro block: %i\nMotion Vector: x_width = %i, y_height = %i\nSAD-value: %f\n", i, x_width_motion, y_height_motion, minimal_SAD));
#endif
        tPixel_index motion_vector = {x_width_motion, y_height_motion};
        tMacro_Block_SAD * macro_block_SAD = (tMacro_Block_SAD *) malloc(sizeof(tMacro_Block_SAD));
        macro_block_SAD->value_SAD = minimal_SAD;
        macro_block_SAD->motion_vector = motion_vector;
        append_element(all_macro_block_SAD, macro_block_SAD);

        // float progress = (i / amount_macro_blocks) * 100;
        // if(progress == 10 || progress == 20 || progress == 30 || progress == 40 || progress == 50 || progress == 60 || progress == 70 || progress == 80 || progress == 90 || progress == 100){
        //     time_printf(("Progress comparing %s and %s: %f\%\n", ref_picture->file_name, other_picture->file_name, progress));
        // }
    }

    return all_macro_block_SAD;
}

//===================En-/Decode Functions===================
int encode_files(tList * file_data, tList * compared_pictures){
    //All data will be seperated by a single space character
    int i;
    //Get the reference picture and data
    tFile_data * ref_picture = (tFile_data *) get_element(file_data, 0)->item;

    for(i = 0; i < compared_pictures->size; i++){
        //Get the current picture to encode
        tFile_data * current_picture = (tFile_data *) get_element(file_data, i + 1)->item;

        char file_name[strlen(current_picture->file_name) + 4];
        strcpy(file_name, current_picture->file_name);
        strcat(file_name, ".bpg");
        xprintf(("file_name: %s\n", file_name));
        FILE * file = fopen(file_name, "w+");

        //Write the amount of macroblocks to the file
        int amount_macro_blocks = get_amount_macro_blocks(current_picture);
        fwrite(&amount_macro_blocks, sizeof(int), 1, file);

        //Get all macro blocks data
        tList * current_all_macro_blocks = (tList *) get_element(compared_pictures, i)->item;
        //Write all motion vectors to the file
        int iterator_motion_vectors;
        for(iterator_motion_vectors = 0; iterator_motion_vectors < current_all_macro_blocks->size; iterator_motion_vectors++){
            tMacro_Block_SAD * tmp_block_info = (tMacro_Block_SAD *) get_element(current_all_macro_blocks, iterator_motion_vectors)->item;
            tPixel_index current_motion_vector = tmp_block_info->motion_vector;

            fwrite(&current_motion_vector.x_width, sizeof(int), 1, file);
            fwrite(&current_motion_vector.y_height, sizeof(int), 1, file);
        }
        xprintf(("Wrote motion vectors\n"));

        //Write all decoded data
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
                    
                    //Getting the difference as a signed short
                    //Difference is always ref_pixeld_data - encode_pixel_data
                    signed short red_short = (signed short) (tmp_ref_data.red - tmp_current_data.red);
                    signed short green_short = (signed short) (tmp_ref_data.green - tmp_current_data.green);
                    signed short blue_short = (signed short) (tmp_ref_data.blue - tmp_current_data.blue);

                    tEncode_pixel_data * resulting_data = malloc(sizeof(tEncode_pixel_data));
                    resulting_data->red = red_short;
                    resulting_data->green = green_short;
                    resulting_data->blue = blue_short;

                    fwrite(resulting_data, sizeof(tEncode_pixel_data), 1, file);
                    
                    free(resulting_data);
                }
            }
        }
    }

    return EXIT_SUCCESS;
}

//===================End Programm Functions===================
void end_programm(tList * file_data_list, tList * list_compared_pictures){
    int i;

    //Delete all file datas
    tList_Element * current_element = file_data_list->first_element;
    for(i = 0; i < file_data_list->size; i++){
        //Delete the tFile_data->data first
        free(((tFile_data *) current_element->item)->data);
        if(current_element->next_element != NULL){
            current_element = current_element->next_element;
        }
    }
    //Now delete the whole list
    delete_list(file_data_list);

    //Delete all macro block data of compared pictures
    current_element = list_compared_pictures->first_element;
    for(i = 0; i < list_compared_pictures->size; i++){
        delete_list((tList *) current_element->item);
        if(current_element->next_element != NULL){
            current_element = current_element->next_element;
        }
    }

    //Now delete the whole list (again)
    delete_list(list_compared_pictures);
}