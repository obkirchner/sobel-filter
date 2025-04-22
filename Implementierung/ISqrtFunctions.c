#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>
#include <stdint.h>

//based on the works of James Ulery: http://www.azillionmonkeys.com/qed/ulerysqroot.pdf
//builds integer sqrt bit by bit
//checks for every bit if result would be to big, if so discards it, otherwise uses it

//easier to understand algorithm with multiplication operation
uint8_t isqrt_shifting(uint16_t input){
    uint8_t res = 0;
    uint8_t currentBit = 0x80;
    for(uint8_t i= 0; i<8; i++){
        uint8_t tmp = res + currentBit;
        uint16_t sqr = tmp*tmp;
        if(input >= sqr){
            res = tmp;
        }
        currentBit= currentBit >> 1;
    }
    return res;
}
//copy of James Ulery's algorithm adapted for 16bit input
//works around multiplication operation
uint8_t isqrt_shifting_withoutMul(uint16_t input){
    uint8_t res = 0;
    uint16_t tmp = 0;
    uint8_t currentBit = 0x80;
    for(int8_t i= 7; i>=0; i--){
        if(input >= (tmp = (((res<<1)+currentBit)<<i))){
            res += currentBit;
            input -= tmp;
        }
        currentBit = currentBit >> 1;
    }
    return (uint8_t)res;
}
void isqrt_shifting_noMul_SIMD(int arrSize, const uint16_t* restrict inputArray, uint8_t* restrict output){
    int i = 0;
    uint16_t curBitMask[8] = {0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080};
    uint8_t conversionMask[16] = {0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
    __m128i shuffleMask = _mm_loadu_si128((__m128i*)conversionMask);
    for (; i < arrSize - (arrSize % 8); i += 8)
    {
        __m128i tmp;
        __m128i res = _mm_setzero_si128();
        __m128i bits = _mm_loadu_si128((__m128i*) curBitMask);
        __m128i input = _mm_loadu_si128((__m128i*) (inputArray+i));
        for (int8_t n = 7; n >= 0; n--){
            // tmp = (((res<<1)+currentBit)<<n)
            tmp = _mm_slli_epi16(res, 1);
            tmp = _mm_add_epi16(tmp, bits);
            tmp = _mm_slli_epi16(tmp, n);
            // if block 
            __m128i cmpMask =  _mm_cmpeq_epi16(_mm_max_epu16(input, tmp), input);
            __m128i maskedAdd = _mm_and_si128(bits, cmpMask);
            __m128i maskedSub = _mm_and_si128(tmp, cmpMask);
            res = _mm_add_epi16(res, maskedAdd);
            input = _mm_sub_epi16(input, maskedSub);
            // currentBit>>1
            bits = _mm_srli_epi16(bits, 1);
        }
        // convert epi16 to epi8
        res = _mm_shuffle_epi8(res, shuffleMask);
        // stores values
        _mm_storeu_si64(output+i, res);
    }
    for(;i < arrSize; i++){
        output[i] = isqrt_shifting_withoutMul(inputArray[i]);
    }
}

void isqrt_shifting_Mul_SIMD(int arrSize, const uint16_t* restrict inputArray, uint8_t* restrict output){
    int j = 0;
    uint16_t curBitMaskMul[8] = {0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080};
    uint8_t conversionMaskMul[16] = {0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
    __m128i shuffleMaskMul = _mm_loadu_si128((__m128i*)conversionMaskMul);
    for (; j < arrSize - (arrSize % 8); j += 8)
    {
        __m128i tmp;
        __m128i res = _mm_setzero_si128();
        __m128i bits = _mm_loadu_si128((__m128i*) curBitMaskMul);
        __m128i input = _mm_loadu_si128( (__m128i*) (inputArray + j));

        for(int8_t n= 0; n<8; n++){
            //tmp = (res+currentBit)
            tmp = _mm_add_epi16(res, bits);
            //tmp^2
            __m128i sqr = _mm_mullo_epi16(tmp, tmp);
            //if block
            __m128i mask = _mm_cmpeq_epi16(_mm_max_epu16(input, sqr), input);
            tmp = _mm_and_si128(mask, tmp);
            res = _mm_andnot_si128(mask, res);
            res = _mm_or_si128(res, tmp);
            //currentBit>>1
            bits = _mm_srli_epi16(bits, 1);
        }

        // convert epi16 to epi8
        res = _mm_shuffle_epi8(res, shuffleMaskMul);
        // stores values
        _mm_storeu_si64(output+j, res);     
    }
    for(;j < arrSize; j++){
        output[j] = isqrt_shifting_withoutMul(inputArray[j]);
    }
}