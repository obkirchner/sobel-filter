#include "sobel.h"

//filter Functions

//computes sqrt(x^2 + y^2) for all points x from Qh and y from Qv with x and y being at the same coordinates
//using isqrt shifting without mul algorithm
void finalCalculation(size_t size, uint8_t *dest, const uint8_t* restrict sourceHorizontal, const uint8_t* restrict sourceVertical){
    size_t i = 0;
    uint16_t curBitMask[8] = {0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080};
    uint8_t conversionMask[16] = {0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
    __m128i shuffleMask = _mm_loadu_si128((__m128i *) conversionMask);
    for (; i < size - (size % 8); i += 8){
        // loads 8 values from storage and converts to 16bit values -> needed for multiplication
        __m128i horizValues = _mm_cvtepu8_epi16(_mm_loadu_si64((__m128i *)(sourceHorizontal + i)));
        __m128i vertValues = _mm_cvtepu8_epi16(_mm_loadu_si64((__m128i *)(sourceVertical + i)));
        // squares both values
        horizValues = _mm_mullo_epi16(horizValues, horizValues);
        vertValues = _mm_mullo_epi16(vertValues, vertValues);
        // adds values up, with saturation -> if Overflow occures sets to 0xFFFF
        __m128i input = _mm_adds_epu16(horizValues, vertValues);
        // parallel isqr_shifting_withoutMul algorithm
        __m128i tmp;
        __m128i res = _mm_setzero_si128();
        __m128i bits = _mm_loadu_si128((__m128i*) curBitMask);
        for (uint8_t n = 0; n < 8; n++){
            // tmp = (res+currentBit)
            tmp = _mm_add_epi16(res, bits);
            // tmp^2
            __m128i sqr = _mm_mullo_epi16(tmp, tmp);
            // if block
            //unsigned compare greater equal 16bit
            __m128i mask = _mm_cmpeq_epi16(_mm_max_epu16(input, sqr), input);
            //lets through all values for which input>=val^2
            tmp = _mm_and_si128(mask, tmp);
            //retains all other values in their original state
            res = _mm_andnot_si128(mask, res);
            //merges both 
            res = _mm_or_si128(res, tmp);
            // currentBit>>1
            bits = _mm_srli_epi16(bits, 1);
        }
        // convert 16bit to 8 bit using shuffle Mask
        res = _mm_shuffle_epi8(res, shuffleMask);
        // stores values
        _mm_storeu_si64(dest + i, res);
    }
    //rest is done with pure c implementation
    for (; i < size; i++){
        uint32_t tmp = ((sourceHorizontal[i] * sourceHorizontal[i]) + (sourceVertical[i] * sourceVertical[i]));
        if(tmp > UINT16_MAX){
            dest[i] = UINT8_MAX;
            continue;
        } 
        dest[i] = isqrt_shifting_withoutMul((uint16_t) tmp);
    }
}

//border Correction functions

void borderCorrectionHorizontal(size_t height, size_t width, uint8_t* restrict dest,  const uint8_t* restrict img){
    //first row is always 0
    for(size_t i = 0; i < width; i++){
        dest[i] = 0;
    }
    int16_t tmp;
    //computes value at index width -> second row first value
    tmp =  2 * img[0] + img[1] - 2 * img[2 * width] -img[2 * width + 1];
    if(tmp < 0) tmp = 0;
    if(tmp > UINT8_MAX) tmp = UINT8_MAX;
    dest[width] = (uint8_t) tmp;
    //last row has to be done seperately
    //cant start at width because then width-1 could have wrong mem accesses
    for(size_t i = 2 * width; i < (height - 1) * width; i += width){
        //first does right border of row before
        tmp = 2 * img[i - width - 1] + img[i - width - 2] - 2 * img[i + width - 1] - img[i + width - 2];
        if(tmp < 0) tmp = 0;
        if(tmp > UINT8_MAX) tmp = UINT8_MAX;
        dest[i - 1] = tmp;
        //left border of row we are at
        tmp = 2 * img[i - width] + img[i - width + 1] - 2 * img[i + width] - img[i + width + 1];
        if(tmp < 0 ) tmp = 0;
        if(tmp > UINT8_MAX) tmp = UINT8_MAX;
        dest[i] = (uint8_t) tmp;
    }
    //second to last row right border ((height-1)*width-1)
    tmp = 2 * img[(height - 2) * width - 1] +img[(height - 2) * width - 2] - 2 * img[height * width - 1] - img[height * width - 2];
    if(tmp > UINT8_MAX) tmp = UINT8_MAX;
    dest[(height - 1) * width - 1] = (uint8_t)tmp; 
    //left lower corner (at (height-1)*width)
    tmp = 2 * img[(height - 2) * width] + img[(height - 2) * width + 1];
    if(tmp > UINT8_MAX) tmp = UINT8_MAX;
    dest[(height - 1) * width] = (uint8_t) tmp; 
    //last row without corners
    for(size_t i = (height - 1) * width + 1; i < height * width - 1; i++){
        tmp = img[i - width - 1] + 2 *img[i - width] + img[i - width + 1];
        if(tmp > UINT8_MAX) tmp = UINT8_MAX;
        dest[i] = (uint8_t) tmp;
    }
    //right lower corner
    tmp = 2 * img[(height - 1) * width - 1] + img[(height - 1) * width - 2];
    if(tmp > UINT8_MAX) tmp = UINT8_MAX;
    dest[height * width - 1] = (uint8_t) tmp; 
}
void borderCorrectionVertical(size_t height, size_t width, uint8_t* restrict dest, const uint8_t* restrict img){
    //top left corner is always 0
    *dest = 0;
    int16_t tmp;
    //first row without corners
    for(size_t i = 1; i < width - 1; i++){
        tmp =  2 * img[i - 1] + img[i + width - 1] - 2 * img[i + 1] - img[i + width + 1];
        if(tmp < 0) tmp = 0;
        if(tmp > UINT8_MAX) tmp = UINT8_MAX;
        dest[i] = (uint8_t) tmp;
    }
    //right upper corner
    tmp = 2 * img[width - 2] + img[2 * width - 2];
    if (tmp > UINT8_MAX) tmp = UINT8_MAX;
    dest[width - 1] = (uint8_t) tmp;
    //left border always 0
    dest[width] = 0;
    for(size_t i = 2 * width; i < (height - 1) * width; i += width){
        //left border
        dest[i] = 0;
        //right border of row before
        tmp = 2 * img[i - 2] + img[i - width - 2] + img[i + width - 2];
        if(tmp > UINT8_MAX) tmp = UINT8_MAX;
        dest[i - 1] = (uint8_t) tmp;
    }
    //second to last row right border ((height-1)*width-1)
    dest[(height - 1) * width - 1] = 2 * img[(height - 1) * width - 2] + img[(height - 2) * width - 2] + img[height * (width) - 2];
    //left lower corner
    dest[(height - 1) * width] = 0;
    //last row without corners
    for(size_t i = (height - 1) * width + 1; i < height * width - 1; i++){
        tmp = 2 * img[i - 1] + img[i - width - 1] - 2 * img[i + 1] - img[i - width + 1];
        if(tmp < 0) tmp = 0;
        if(tmp > UINT8_MAX) tmp = UINT8_MAX;
        dest[i] = (uint8_t) tmp;
    }
    //right lower corner
    tmp = 2 * img[height * width - 2] + img[(height - 1) * width - 2];
    if(tmp < 0) tmp = 0;
    if(tmp > UINT8_MAX) tmp = UINT8_MAX;
    dest[height * width - 1] = (uint8_t) tmp;
}

//Inner data functions

void innerDataHorizontal(size_t width, size_t size, uint8_t* restrict dest, const uint8_t* restrict img){
    //start at width +1 to prevent negative indeces (upper row and the value at second row left border have to be done seperately)
    //end at size-width-1 to prevent invalid indeces (bigger then array) (lowest row and value at the right border of the second to last row have to be done seperately).
    //mod 8 ensures we end after the full 8 value block
    //8byte per loop because we need to manage overflows
    //borders are getting computed but need to be corrected otherwise 
    __m128i zeroReg = _mm_setzero_si128();
    uint16_t mask[8] = {255,255,255,255,255,255,255,255};
    __m128i upperBorder = _mm_loadu_si128((__m128i*) mask);
    uint8_t conversionMask[16] = {0x00,0x02,0x04,0x06,0x08,0x0a,0x0c,0x0e,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80};
    __m128i shuffleMask = _mm_loadu_si128((__m128i*) conversionMask);
    size_t i = width + 1;
    for(; i < (size-width-1)-((size-width-1)%8); i+=8){
        // 0,0 1,0 2,0
        // 0,1 1,1 2,1
        // 0,2 1,2 2,2 
        // -> the pixel we are calculating is at (1,1)
        //load (1,0)=x and calculate 2*x
        __m128i res = _mm_loadu_si64((__m128i*) (img + i - width));
        res = _mm_cvtepu8_epi16(res);
        res = _mm_slli_epi16(res, 1);
        //load (1,2)=y and calculate 2*y
        __m128i lower = _mm_loadu_si64((__m128i*) ((img + i) + width));
        lower = _mm_cvtepu8_epi16(lower);
        lower = _mm_slli_epi16(lower, 1);
        //x-y
        res = _mm_sub_epi16(res, lower);
        //load (0,0) and add to result
        __m128i higher = _mm_loadu_si64((__m128i*) ((img + i) - width - 1));
        higher = _mm_cvtepu8_epi16(higher);
        res = _mm_add_epi16(res, higher);
        //load (2,0) and add to result
        higher = _mm_loadu_si64((__m128i*) ((img + i) - width + 1));
        higher = _mm_cvtepu8_epi16(higher);
        res = _mm_add_epi16(res, higher);
        //load (0,2) and subtract from result
        lower = _mm_loadu_si64((__m128i*) ((img + i) + width - 1));
        lower = _mm_cvtepu8_epi16(lower);
        res = _mm_sub_epi16(res, lower);
        //load (2,2) and subtract from result
        lower = _mm_loadu_si64((__m128i*) ((img + i) + width + 1));
        lower = _mm_cvtepu8_epi16(lower);
        res = _mm_sub_epi16(res, lower);
        //check for <0 and >255
        __m128i mask = _mm_cmpgt_epi16(res, zeroReg);
        __m128i upperMask = _mm_cmpgt_epi16( res, upperBorder);
        res = _mm_and_si128(res, mask);
        res = _mm_or_si128(res, upperMask);
        //convert 16bit values to 8bit values using shuffleMask
        res = _mm_shuffle_epi8(res, shuffleMask);
        //store
        _mm_storeu_si64(dest + i, res);
    }
    int16_t tmp;
    for(; i < (size - width - 1);i++){
        tmp = 2 * img[i - width] + img[i - width - 1] + img[i - width + 1] - 2 * img[i + width] - img[i + width - 1] - img[i + width + 1];
        if(tmp < 0) tmp = 0;
        if(tmp > UINT8_MAX) tmp = UINT8_MAX;
        dest[i] = (uint8_t) tmp;
    }
}

void innerDataVertical(size_t width, size_t size, uint8_t* restrict dest, const uint8_t* restrict img){
    //start at width +1 to prevent negative indeces (upper row and the value at second row left border have to be done seperately)
    //end at size-width-1 to prevent invalid indeces (bigger then array) (lowest row and value at the right border of the second to last row have to be done seperately).
    //mod 8 ensures we end after the full 8 value block
    //8byte per loop because we need to manage overflows
    //borders are getting computed but need to be corrected otherwise 
    size_t i = width + 1;
    __m128i zeroReg = _mm_setzero_si128();
    uint16_t mask[8] = {255,255,255,255,255,255,255,255};
    __m128i upperBorder = _mm_loadu_si128((__m128i*) mask);
    uint8_t conversionMask[16] = {0x00,0x02,0x04,0x06,0x08,0x0a,0x0c,0x0e,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80};
    __m128i shuffleMask = _mm_loadu_si128((__m128i*) conversionMask);
    for(; i < (size - width - 1) - ((size - width - 1)%8); i += 8){
        // 0,0 1,0 2,0
        // 0,1 1,1 2,1
        // 0,2 1,2 2,2 
        // -> the pixel we are calculating is at (1,1)
            // load (0,0) and (2,0) and sub (2,0) from (0,0)
            __m128i upper = _mm_loadu_si64((__m128i*) (img + i - 1 - width));
            __m128i shifted = _mm_loadu_si64((__m128i*) (img + i + 1 - width));
            upper = _mm_cvtepu8_epi16(upper);
            shifted = _mm_cvtepu8_epi16(shifted);
            upper = _mm_sub_epi16(upper, shifted);
            // load (0,1)=x and (2,1)=y and calculate 2x-2y 
            __m128i mid = _mm_loadu_si64((__m128i*) (img + i - 1));
            shifted = _mm_loadu_si64((__m128i*) (img +i + 1));
            mid = _mm_cvtepu8_epi16(mid);
            shifted = _mm_cvtepu8_epi16(shifted);
            mid = _mm_slli_epi16(mid, 1);
            shifted = _mm_slli_epi16(shifted, 1);
            mid = _mm_sub_epi16(mid, shifted);
            // load (0,2) and (2,2) and subtract them
            __m128i lower = _mm_loadu_si64((__m128i*) (img + i - 1 + width));
            shifted = _mm_loadu_si64((__m128i*) (img + i + 1 + width));
            lower = _mm_cvtepu8_epi16(lower);
            shifted = _mm_cvtepu8_epi16(shifted);
            lower = _mm_sub_epi16(lower, shifted);
            //add values and check for < 0 or > 255
            mid = _mm_add_epi16(mid, _mm_add_epi16(upper, lower));
            __m128i mask = _mm_cmpgt_epi16(mid, zeroReg);
            __m128i upperMask = _mm_cmpgt_epi16(mid, upperBorder);
            mid = _mm_and_si128(mid, mask);
            mid = _mm_or_si128(mid, upperMask);
            //convert 16bit to 8 bit values using shuffleMask
            mid = _mm_shuffle_epi8(mid, shuffleMask);
            //store
            _mm_storeu_si64(dest + i, mid);
    }
    int16_t tmp;
    //remaining rest with pure C
     for(; i < (size - width - 1); i++){
        tmp = img[i - width - 1] + 2 * img[i - 1] + img[i + width - 1] - 2 * img[i + 1] - img[i + width + 1] - img[i - width + 1];
        if(tmp < 0) tmp = 0;
        if(tmp> UINT8_MAX) tmp = UINT8_MAX;
        dest[i] = (uint8_t) tmp;
     }
}

//main function
void sobelFilter_V0(const uint8_t* img, size_t width, size_t height, uint8_t* result){
    //allocating Qv and Qh (vertical and horizontal temporary Matrix)
    size_t size = height * width;
    uint8_t* horizontalMatrix = (uint8_t*) malloc(size);
    if(!horizontalMatrix){ 
        fprintf(stderr, "Error: Couldnt allocate storage for Horizontal Matrix in sobelFilter_V0 (l. 282)");
        exit(EXIT_FAILURE);
    }
    uint8_t* verticalMatrix = (uint8_t*) malloc(size);
    if(!verticalMatrix){ 
        fprintf(stderr, "Error: Couldnt allocate storage for Vertical Matrix in sobelFilter_V1 (l. 287)");
        free(horizontalMatrix);
        exit(EXIT_FAILURE);
    }
    //split in inner and outer data as to not have to do borderchecks every loop
    innerDataHorizontal(width, size, horizontalMatrix, img);
    borderCorrectionHorizontal(height, width, horizontalMatrix, img);

    innerDataVertical(width, size, verticalMatrix, img);
    borderCorrectionVertical(height, width, verticalMatrix, img);

    //clamping operation
    finalCalculation(size, result, horizontalMatrix, verticalMatrix);

    free(horizontalMatrix);
    free(verticalMatrix);
}