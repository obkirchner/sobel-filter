#ifndef SQRT
#define SQRT
#include <stdint.h>
uint8_t isqrt_shifting(uint16_t input);
uint8_t isqrt_shifting_withoutMul(uint16_t input);
void isqrt_shifting_noMul_SIMD(int arrSize, uint16_t* inputArray, uint8_t* output);
void isqrt_shifting_Mul_SIMD(int arrSize, uint16_t* inputArray, uint8_t* output);
#endif