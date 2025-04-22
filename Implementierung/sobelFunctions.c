#include "sobel.h"
//main implementation

void sobel(const uint8_t* img, size_t width, size_t height, float a, float b, float c, void* tmp, uint8_t* result){
    greyscale_abc_V0(img, tmp, width, height, a, b, c);
    sobelFilter_V0(tmp, width, height, result);
}

void sobel_V1(const uint8_t* img, size_t width, size_t height, float a, float b, float c, void* tmp, uint8_t* result){
    greyscale_abc_V0(img, tmp, width, height, a, b, c);
    sobelFilter_V1(tmp, width, height, result);
}