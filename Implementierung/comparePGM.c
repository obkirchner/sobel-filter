#include "sobel.h"
#include <inttypes.h>
//to compile: gcc comparePGM.c correct.h PGM_Image.h readPGM.c -std=gnu99 -O3 -Wall -Wextra -Wpedantic -g -o testCorrect.out

//Compares two images with same width and height
void comparePGM(const uint8_t* first, const uint8_t* second, size_t width, size_t height){
    uint32_t numberOfDeviations = 0;
    int magnOfDeviation = 0;
    uint8_t highestDeviation = 0;
    size_t size = height*width;
    for(size_t i = 0; i < size; i++){
        if(first[i] != second[i]){
            numberOfDeviations++;
            uint8_t dev = abs(first[i] - second[i]);
            if(dev > highestDeviation) highestDeviation = dev;
            magnOfDeviation += (int) dev;
        
        }
    }
    float percentageOfDeviations = (float)numberOfDeviations/size;
    float averageDeviation  = (float) magnOfDeviation/numberOfDeviations;
    printf("Number of Deviations detected: %"PRIu32"\n", numberOfDeviations);
    printf("Percentage of Pixels with deviations: %f \n", percentageOfDeviations);
    printf("Highest Deviation detected: %"PRIu8"\n", highestDeviation);
    printf("Average Deviation detected: %f\n", averageDeviation);
}

int main(int argc, char* argv[]){
    size_t width = 1280;
    size_t height = 853;
    uint8_t* input = malloc(height*width*3);
    if(!input){
        perror("Error allocatin Storage for testing input array");
        exit(EXIT_FAILURE);
    }
    generate(input, width, height);
    void* tmp = malloc(height*width);
    if(!tmp){
        free(input);
        perror("Error allocatin Storage for testing tmp array");
        exit(EXIT_FAILURE);
    }
    uint8_t* first = malloc(height*width);
    if(!first){
        free(input);
        free(tmp);
        perror("Error allocatin Storage for testing first array");
        exit(EXIT_FAILURE);
    }
    uint8_t* second = malloc(height*width);
    if(!second){
        free(input);
        free(tmp);
        free(first);
        perror("Error allocatin Storage for testing second array");
        exit(EXIT_FAILURE);
    }
    sobel(input, width, height, 1, 1, 1, tmp, first);
    sobel_V1(input, width, height, 1, 1, 1, tmp, second);
    comparePGM(first, second, width, height);
    free(input);
    free(tmp);
    free(first);
    free(second);
    exit(EXIT_SUCCESS);
}