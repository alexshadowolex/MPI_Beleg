#include "functions.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb/stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb/stb_image.h"

#define STB_DEFINE
#include "../lib/stb/stb.h"

//#define PNGSUITE_PRIMARY
#define TERM_OUTPUT


//#define X11_DISPLAY


//Globally uses vars


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

                // xprintf(("Data: %c\n", data[tmp_height * width * 4 + tmp_width * 4]));
                xprintf(("pixel %03d,%03d:  rgb: %03d,%03d,%03d \n", tmp_height, tmp_width, (int) red, (int) green, (int) blue));
            }
        }         

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
        int size_tmp = sizeof(file_name) + sizeof(data) + (sizeof(height) / sizeof(int)) + (sizeof(width) / sizeof(int));
        xprintf(("Size tmp: %i\n", size_tmp));
        tmp = malloc(size_tmp);
        tmp->file_name = file_name;
        tmp->data = data;
        tmp->height = height; 
        tmp->width = width;
        // free(data);	    

    } else {
        printf("Failed loading data on second try, picture %s\n", file_name);
    }
    return tmp;
}

//===================SAD Functions===================

float calculate_SAD(tFile_data * data_ref_picture, tFile_data * data_other_picture){
    float SAD = 0;
    int i, j;
    xprintf(("%s\n%s\n", data_ref_picture->data, data_other_picture->data));
    //TODO Things dont work :(
    for(i = 0; i < data_ref_picture->width; i++){
        for(j = 0; j < data_ref_picture->height; j++){
            int access_index = i * data_ref_picture->width * 4 + j * 4;
            xprintf(("Accessing element at index %i\n", access_index));
            SAD = SAD + abs(data_other_picture->data[access_index] - data_ref_picture->data[access_index]);
        }
    }
    return SAD;
}