#include "functions.h"
#include <time.h>
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
    struct tm* tm_info;
    struct timeval tv;

    gettimeofday(&tv, NULL);

    millisec = lrint(tv.tv_usec/1000.0); // Round to nearest millisec
    if (millisec>=1000) { // Allow for rounding up to nearest second
    millisec -=1000;
    tv.tv_sec++;
    }

    tm_info = localtime(&tv.tv_sec);


    strftime(buffer, 26, "%H:%M:%S", tm_info);
    printf("%s.%03d ", buffer, millisec);
}

//===================Reader Functions===================
#ifdef X11_DISPLAY
// #include    <../lib/X11/Xlib.h>
XImage *CreateTrueColorImage(Display *display, Visual *visual, unsigned char *image, int width, int height, unsigned char *data){
    int i, j;
    unsigned char *image32=(unsigned char *)malloc(width*height*4);
    unsigned char *p=image32;
    for(i=0; i<width; i++){
        for(j=0; j<height; j++){	    
            *p++ = data[i*height*4+j*4+0];
            *p++ = data[i*height*4+j*4+1];
            *p++ = data[i*height*4+j*4+2];
            *p++;
        }
    }
    return XCreateImage(display, visual, 24, ZPixmap, 0, image32, width, height, 32, 0);
}

void processEvent(Display *display, Window window, XImage *ximage, int width, int height){
    static char *tir="This is red";
    static char *tig="This is green";
    static char *tib="This is blue";
    XEvent ev;
    XNextEvent(display, &ev);
    switch(ev.type)
    {
        case Expose:
            XPutImage(display, window, DefaultGC(display, 0), ximage, 0, 0, 0, 0, width, height);
            /* XSetForeground(display, DefaultGC(display, 0), 0x00ff0000); // red
            XDrawString(display, window, DefaultGC(display, 0), 32,    32,    tir, strlen(tir));
            XDrawString(display, window, DefaultGC(display, 0), 32+256, 32,    tir, strlen(tir));
            XDrawString(display, window, DefaultGC(display, 0), 32+256, 32+256, tir, strlen(tir));
            XDrawString(display, window, DefaultGC(display, 0), 32,    32+256, tir, strlen(tir));
            XSetForeground(display, DefaultGC(display, 0), 0x0000ff00); // green
            XDrawString(display, window, DefaultGC(display, 0), 32,    52,    tig, strlen(tig));
            XDrawString(display, window, DefaultGC(display, 0), 32+256, 52,    tig, strlen(tig));
            XDrawString(display, window, DefaultGC(display, 0), 32+256, 52+256, tig, strlen(tig));
            XDrawString(display, window, DefaultGC(display, 0), 32,    52+256, tig, strlen(tig));
            XSetForeground(display, DefaultGC(display, 0), 0x000000ff); // blue
            XDrawString(display, window, DefaultGC(display, 0), 32,    72,    tib, strlen(tib));
            XDrawString(display, window, DefaultGC(display, 0), 32+256, 72,    tib, strlen(tib));
            XDrawString(display, window, DefaultGC(display, 0), 32+256, 72+256, tib, strlen(tib));
            XDrawString(display, window, DefaultGC(display, 0), 32,    72+256, tib, strlen(tib));
            */
            break;
        case ButtonPresults:
            exit(EXIT_FAILURE);
    }
}

#endif



tFile_data * read_picture(char * file_name){

#ifdef X11_DISPLAY
    XImage *ximage;
    Display *display;
    Visual *visual;
    Window window;
#endif
    int result;
    int width, height;
    int w2,h2,n2;
    int tmp_height, tmp_width;
    int n;
    unsigned char * data;
    tFile_data * tmp;
    result = stbi_info(file_name, &w2, &h2, &n2);
    // xprintf(("stb_info(%s, &w2, &h2, &n2) --> w2:%d, h2:%d, n2:%d\n", file_name, w2, h2, n2));

    data = stbi_load(file_name, &width, &height, &n, 4); 
    if (data){ 
        free(data);
    } else {
        printf("Failed loading data of picture %s\n", file_name);
    }
    /*
    data = stbi_load(file_name, &width, &height,  0, 1); if (data) free(data); else printf("Failed 1\n");
    data = stbi_load(file_name, &width, &height,  0, 2); if (data) free(data); else printf("Failed 2\n");
    data = stbi_load(file_name, &width, &height,  0, 3); if (data) free(data); else printf("Failed 3\n");
    */


    // load image once again and check whether the same values are obtained 
    data = stbi_load(file_name, &width, &height, &n, 4);
    assert(data);
    assert(width == w2 && height == h2 && n == n2);
    assert(result);

    if (data) {
        char fname[512];
        stb_splitpath(fname, file_name, STB_FILE);
        /* 
        stbi_write_png(stb_sprintf("output/%s.png", fname), width, height, 4, data, width*4);
        stbi_write_bmp(stb_sprintf("output/%s.bmp", fname), width, height, 4, data);
        stbi_write_tga(stb_sprintf("output/%s.tga", fname), width, height, 4, data);
        */
        stbi_write_jpg(stb_sprintf("output/%s_copy.jpg", fname), width, height, 4, data, 100);

        xprintf(("File output for %s completed\n", file_name));

        for (tmp_height = 0; tmp_height < height; tmp_height++ ){
            for (tmp_width = 0; tmp_width < width; tmp_width++){
                unsigned char red, green, blue;

                red =   data[tmp_height * width * 4 + tmp_width * 4 + 0];
                green = data[tmp_height * width * 4 + tmp_width * 4 + 1];
                blue =  data[tmp_height * width * 4 + tmp_width * 4 + 2];

                // xprintf(("Index: %i\n", (tmp_height * width * 4 + tmp_width * 4 + 2)));
                // xprintf(("pixel %03d,%03d:  rgb: %03d,%03d,%03d \n", tmp_height, tmp_width, (int) red, (int) green, (int) blue));
            }
        }    
        // xprintf(("Last index: %i\n", ((height - 1) * width * 4 + (width - 1) * 4 + 2))); 

        xprintf(("Terminal output for %s completed\n", file_name));

#ifdef X11_DISPLAY
        printf("Showing picture on X11 display\n");

        display=XOpenDisplay(NULL);
        visual=DefaultVisual(display, 0);
        window=XCreateSimpleWindow(display, RootWindow(display, 0), 0, 0, width, height, 1, 0, 0);
        if(visual->class!=TrueColor){
            fprintf(stderr, "Cannot handle non true color visual ...\n");
            exit(EXIT_SUCCESS);
        }

        ximage=CreateTrueColorImage(display, visual, 0, width, height, data);
        XSelectInput(display, window, ButtonPresultsMask|ExposureMask);
        XMapWindow(display, window);
        while(1){
            processEvent(display, window, ximage, width, height);
        }  

#endif
        tmp = malloc(sizeof(tFile_data));
        tmp->file_name = file_name;
        tmp->data = (char *) malloc((height - 1) * width * 4 + (width - 1) * 4 + 2);
        memcpy(tmp->data, data, (height - 1) * width * 4 + (width - 1) * 4 + 2);
        tmp->height = height; 
        tmp->width = width;
        // free(data);	    

    } else {
        printf("Failed loading data on second try, picture %s\n", file_name);
    }
    return tmp;
}

tPixel_data access_file_data_array(tFile_data * file, int x_width, int y_height){
    int access_index = y_height * file->width * 4 + x_width * 4;
#ifdef TEST_ACCESS
    xprintf(("access_index = %i\n", access_index));
#endif
    unsigned char red_char = file->data[access_index + 0];
    unsigned char green_char = file->data[access_index + 1];
    unsigned char blue_char = file->data[access_index + 2];
    int red = (int) red_char;
    int green = (int) green_char;
    int blue = (int) blue_char;
    if(y_height >= file->height || x_width >= file->width){
        red = -1;
        green = -1;
        blue = -1;
    }

    tPixel_data ret_value = {
        red,
        green,
        blue
    };
    return ret_value;
}

int get_amount_macro_blocks(tFile_data * ref_picture){
    //Since every picture has an integer amount of macro blocks, the calculation is easy
    return (ref_picture->height / 16) * (ref_picture->width / 16);
}

/* Macro blocks are counted like this:
 * 0-1-2
 * 3-4-5
 * 6-7-8
 * The function returns the index of the left-upper pixel of a macro block (first: 0,0; second: 0,16;...)
 */
void get_macro_block_begin(tFile_data * ref_picture, int number_macro_block, int index[]){
    //returns: int[]{width, height}
    if(number_macro_block >= get_amount_macro_blocks(ref_picture)){
        return;
    }
    int blocks_per_line = ref_picture->width / 16;
    int line = number_macro_block / blocks_per_line;
    int col = number_macro_block % blocks_per_line;

    int pixel_width = col * 16;
    int pixel_height = line * 16;

    index[0] = pixel_width;
    index[1] = pixel_height;
}

//===================SAD Functions===================
//Für jeden makroblock beim Testen des passenden verschiebungsvektor den jeweiligen sad-werte zu dem vektor speichern (struct?)
//und nach beendigung alle werte vergleichen und den kleinsten nehmen und in eine liste speichern
//Funktion kann frühzeitig beendt werden, wenn der Wert 0 gefunden wurde oder ein wert über dem aktuellen minimum liegt.

tList * calc_SAD_values(tFile_data * ref_picture, tFile_data * other_picture, int distanze_motion_vector_search){
    //The list holds for all macro blocks the best motion vector in a struct tMacro_Block_SAD
    tList * all_macro_block_SAD = create_list();
    int amount_macro_blocks = get_amount_macro_blocks(ref_picture);
    int i, current_x_width_motion, current_y_height_motion, x_current_width_macro_block, y_current_height_macro_block;
// #ifdef TEST_SAD_CALC
    xprintf(("Distance motion vector: %i\n", distanze_motion_vector_search));
// #endif
    for(i = 0; i < amount_macro_blocks; i++){
        //Compare all macro blocks
        int begin_index[2];
        get_macro_block_begin(ref_picture, i, begin_index);
        float minimal_SAD = INT_MAX;
        int x_width_motion;
        int y_height_motion;
        int found_minimal_SAD = 0;
        for(current_x_width_motion = (-1) * distanze_motion_vector_search; 
            current_x_width_motion <= distanze_motion_vector_search && !found_minimal_SAD; 
            current_x_width_motion++){
            for(current_y_height_motion = (-1) * distanze_motion_vector_search; 
                current_y_height_motion <= distanze_motion_vector_search && !found_minimal_SAD; 
                current_y_height_motion++){
                float current_SAD = 0;
                //Calculate minimal SAD and save the value and the fitting distance motion vector
                //Iterate over all pixels in a macro block (16x16)
                for(x_current_width_macro_block = begin_index[0]; x_current_width_macro_block < begin_index[0] + 16; x_current_width_macro_block++){
                    for(y_current_height_macro_block = begin_index[1]; y_current_height_macro_block < begin_index[1] + 16; y_current_height_macro_block++){
                        //Get current pixeldata from ref_picture
                        tPixel_data ref_pixel = access_file_data_array(ref_picture, x_current_width_macro_block, y_current_height_macro_block);
                        //Get current pixeldata from other_picture, moved by current motion vector
                        tPixel_data other_pixel = access_file_data_array(other_picture, x_current_width_macro_block + current_x_width_motion, y_current_height_macro_block + current_y_height_motion);

                        if(other_pixel.red == - 1){
                            //Means we tried to access a pixel outside of the picture
                            continue;
                        }
                        float ref_brightness = 0.30 * ref_pixel.red + 0.59 * ref_pixel.green + 0.11 * ref_pixel.blue;
                        float other_brightness = 0.30 * other_pixel.red + 0.59 * other_pixel.green + 0.11 * other_pixel.blue;
                        current_SAD += abs(ref_brightness - other_brightness);
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
        }
        //Add vector for macro block here
#ifdef TEST_SAD_CALC
        xprintf(("Current macro block: %i\nMotion Vector: x_width = %i, y_height = %i\nSAD-value: %f\n", i, x_width_motion, y_height_motion, minimal_SAD));
#endif
        tMotion_Vector motion_vector = {x_width_motion, y_height_motion};
        tMacro_Block_SAD * macro_block_SAD = (tMacro_Block_SAD *) malloc(sizeof(tMacro_Block_SAD));
        macro_block_SAD->value_SAD = minimal_SAD;
        macro_block_SAD->motion_vector = motion_vector;
        append_element(all_macro_block_SAD, macro_block_SAD);
    }

    return all_macro_block_SAD;
}