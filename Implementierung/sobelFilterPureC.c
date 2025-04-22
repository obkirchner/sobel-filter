#include "sobel.h"


void horizontalFilter(const uint8_t* restrict data, uint8_t* restrict dest,  size_t size, size_t width){
    int32_t tmp;
    for(size_t i = 0; i < size; i++){
        tmp = 0;
        //>= 0 check to not have wrong memory access 
        //i%x checks for border
        // 0,0 1,0 2,0
        // 0,1 1,1 2,1
        // 0,2 1,2 2,2 
        //current Pixel at (1,1)

        //i-width-1 -> (0,0)
        if((((long) i - (long) width - 1) >= 0) && (i % width != 0)){
            tmp += data[i - width - 1];
        }
        //i-width -> (1,0)
        if(((long) i - (long) width) >= 0){
            tmp += 2 * data[i - width];
            }
        //i-width+1 -> (2,0)
        if((((long) i - (long) width + 1) >= 0)  && (i + 1) % width != 0){
            tmp += data[i - width + 1];
        }
        if((((i + width - 1) < size) && (i % width != 0))){
            tmp -= data[i + width - 1];
        }
        if((i + width) < size){
            tmp -= 2 * data[i + width];
        }
        if(((i + width + 1) < size) && (((i + 1) % width) != 0)){
            tmp -= data[i + width + 1];
        }
        if(tmp < 0) tmp = 0;
        if(tmp > UINT8_MAX) tmp = UINT8_MAX;
        dest[i] = (uint8_t) tmp;
    }
}
void verticalFilter(const uint8_t* restrict data, uint8_t* restrict dest,  size_t size, size_t width){
    int16_t tmp;
    for(size_t i = 0; i < size; i++){
        tmp = 0;
        if((((long) i - (long) width -1 ) >= 0) && ((i % width) != 0)){
            tmp += data[i-width -1];
        } 
        if(((long) i - 1 >= 0) && ((i % width) != 0)){
            tmp += 2 * data[i - 1];
        } 
        if((i%width) != 0){
            tmp += data[i + width - 1];
        } 
        if((((long) i - (long) width + 1 >= 0) && ((i - width + 1) < size) && (((i + 1) % width) != 0))){
            tmp -= data[i - width + 1];
        } 
        if(((i + 1) < size) && (((i + 1) % width) != 0)){
            tmp -= 2 * data[i + 1];
        }
        if(((i + width + 1) < size) && (((i + 1) % width) != 0)){
            tmp -= data[i + width + 1];
        }
        if(tmp < 0) tmp = 0;
        if(tmp > UINT8_MAX) tmp = UINT8_MAX;
        dest[i] = (uint8_t) tmp;
    }
}
// uint8_t img* size_t width, size_t height
void sobelFilter_V1(const uint8_t* restrict img, size_t width, size_t height, uint8_t* restrict result){
    size_t size = height * width;
    uint8_t* horizontalMatrix = (uint8_t*) malloc(size);
    if(!horizontalMatrix){
        fprintf(stderr, "Error: Couldnt allocate storage for horizontal Matrix in sobelFilter_V1 (l. 71)");
        exit(EXIT_FAILURE);
    }
    uint8_t* verticalMatrix = (uint8_t*) malloc(size);
    if(!verticalMatrix){
        free(horizontalMatrix);
        fprintf(stderr, "Error: Couldnt allocate storage for vertical Matrix in sobelFilter_V1 (l. 76)");
        exit(EXIT_FAILURE);
    }
    //calculates Qh
    horizontalFilter(img, horizontalMatrix, size, width);
    //calculates Qv
    verticalFilter(img, verticalMatrix, size, width);
    //clamping Operation
    uint32_t tmp;
    for(size_t i = 0; i < size; i++){
        tmp = (horizontalMatrix[i] * horizontalMatrix[i]) + (verticalMatrix[i] * verticalMatrix[i]);
        if(tmp > UINT16_MAX) tmp = UINT16_MAX;
        tmp = (uint32_t) isqrt_shifting_withoutMul(tmp);
        result[i] = (uint8_t) tmp;
    }
    free(horizontalMatrix);
    free(verticalMatrix);
}

