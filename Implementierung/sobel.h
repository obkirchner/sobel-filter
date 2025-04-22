#ifndef SOBEL
#define SOBEL
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <string.h>


void sobelFilter_V0(const uint8_t* img, size_t width, size_t height, uint8_t* result);
void sobelFilter_V1(const uint8_t* img, size_t width, size_t height, uint8_t* result);
void sobel(const uint8_t* img, size_t width, size_t height, float a, float b, float c, void* tmp, uint8_t* result);
void sobel_V1(const uint8_t* img, size_t width, size_t height, float a, float b, float c, void* tmp, uint8_t* result);
uint8_t isqrt_shifting(uint16_t input);
uint8_t isqrt_shifting_withoutMul(uint16_t input);
void isqrt_shifting_noMul_SIMD(int arrSize, const uint16_t* restrict inputArray, uint8_t* restrict output);
void isqrt_shifting_Mul_SIMD(int arrSize, const uint16_t* restrict inputArray, uint8_t* restrict output);
uint8_t* format(FILE* file, size_t* width, size_t* height);
void greyscale_abc_V0 (const uint8_t* input, uint8_t* output, size_t width, size_t height, float a, float b, float c);
void greyscale_V0 (const uint8_t* input, uint8_t* output, size_t width, size_t height);
void grayscaleASM(uint8_t* img, uint8_t* output, size_t size, float a, float b, float c);
void writePGM(FILE* file, const uint8_t* img, size_t width, size_t height);
void generate(uint8_t* img, size_t width, size_t height);

#endif

