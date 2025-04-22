#include "sobel.h"



void generate(uint8_t* img, size_t width, size_t height){
    size_t image_in_bytes = 3*width*height;
    for (size_t i = 0; i < image_in_bytes; i++){
        img[i] = rand();
    }
}