#include "sobel.h"


// grayscale conversion in pure c
void greyscale_abc_V0 (const uint8_t* input, uint8_t* output, size_t width, size_t height, float a, float b, float c) {
    size_t size = height * width;
    size_t j = 0; // counts pixel
    float div = a + b + c;

    // iterates over every pixel
    for(size_t i = 0; i < (3u * size); i = i + 3){
        float tmp = (input[i] * a) + (input[i + 1] * b) + (input[i + 2] * c); // calculates nominater
        *(output + j) = (uint8_t)((tmp / div) + 0.5); // divides the denominator from the nominator
        j++;
    }
}
