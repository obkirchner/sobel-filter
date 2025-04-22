#include "sobel.h"

// writes data from given array in the given file
void writePGM(FILE* file, const uint8_t* image, size_t width, size_t height){
    fprintf(file, "P5\n"); //writes modus
    fprintf(file, "%zu %zu\n", width, height); // writes width and height
    fprintf(file, "255\n"); // writes maximum value
    fwrite(image, width, height, file); // writes the values of the pixels in the file
}