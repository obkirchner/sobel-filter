#define _POSIX_C_SOURCE 199309L
#include "sobel.h"
#include <time.h>
#include <unistd.h>

struct timespec start, end;
double timee = 0.0;
double avg = 0.0;
uint8_t sqrt_res = 0;
uint8_t* result_image;
size_t height = 0;
size_t width = 0;
void* tmp;
//random example values for the greyscale conversion
uint8_t a = 45;
uint8_t b = 63;
uint8_t c = 189;

void test_sqrt(){
    //generate array of 1000000 random values 
    uint16_t* random = (uint16_t*)malloc(1000000*sizeof(uint16_t));
    if(!random){
        fprintf(stderr, "Error allocating memory for random Array");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 1000000; i++){
        random[i] = rand()%65535;
    }
    //necessary for the SIMD functions
    uint8_t* out = (uint8_t*)malloc(1000000*sizeof(uint8_t));
    if(!out){
        free(random);
        fprintf(stderr, "Error allocating memory for out Array");
        exit(EXIT_FAILURE);
    }
    //measure 1000000 times the sqrt functions with random values
    //non-SIMD sqrt with multiplication
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 1000000; i++){
        sqrt_res = isqrt_shifting(random[i]);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    timee = (end.tv_sec - start.tv_sec) * 1.0e9 + (end.tv_nsec - start.tv_nsec);
    avg = timee/1000000;
    printf("\n\n-------------------------sqrt test---------------------------------------------");
    printf ("\n\n average isqrt_shifting in nano sec: %f \n", avg);
    //non-SIMD sqrt without multiplication
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 1000000; i++){
        sqrt_res = isqrt_shifting_withoutMul(random[i]);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    timee = (end.tv_sec - start.tv_sec) * 1.0e9 + (end.tv_nsec - start.tv_nsec);
    avg = timee/1000000;
    printf (" average isqrt_shifting_withoutMul in nano sec: %f \n", avg);
    //SIMD sqrt without multiplication
    clock_gettime(CLOCK_MONOTONIC, &start);
    isqrt_shifting_noMul_SIMD(1000000, random, out);
    clock_gettime(CLOCK_MONOTONIC, &end);
    timee = (end.tv_sec - start.tv_sec) * 1.0e9 + (end.tv_nsec - start.tv_nsec);
    avg = timee/1000000;
    printf (" average isqrt_shifting_noMul_SIMD in nano sec: %f \n", avg);
    //SIMD sqrt with multiplication
    clock_gettime(CLOCK_MONOTONIC, &start);
    isqrt_shifting_Mul_SIMD(1000000, random, out);
    clock_gettime(CLOCK_MONOTONIC, &end);
    timee = (end.tv_sec - start.tv_sec) * 1.0e9 + (end.tv_nsec - start.tv_nsec);
    avg = timee/1000000;
    printf (" average isqrt_shifting_Mul_SIMD in nano sec: %f \n", avg);
    //sqrt from C standard library for comparison
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 1000000; i++){
        sqrt_res = sqrt(random[i]); 
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    timee = (end.tv_sec - start.tv_sec) * 1.0e9 + (end.tv_nsec - start.tv_nsec);
    avg = timee/1000000;
    printf (" average of standard sqrt in nano sec: %f\n", avg);
    printf("\n\n-------------------------greyscale and sobel tests-----------------------------------------\n\n\n");
    printf("*time in sec*\n\n");
    free(random);
    free(out);
}

void test_greyscale_and_sobel(uint8_t* input_image, size_t width, size_t height) {
    printf("width: %zupx, height:%zupx\n", width, height);
    //malloc for tmp and result_image
    void* tmp = malloc(height*width*(sizeof(uint8_t)));
    if(!tmp){
        free(input_image);
        fprintf(stderr, "Error allocating memory for tmp Array");
        exit(EXIT_FAILURE);
    }
    result_image = malloc(height*width*(sizeof(uint8_t)));
    if(!result_image){
        free(input_image);
        free(tmp);
        fprintf(stderr, "Error allocating memory for resultImg Array");
        exit(EXIT_FAILURE);
    }
    //measure the average of 10 function calls per implemenatation
    //greyscale Assembler
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 10; i++){
        grayscaleASM(input_image, result_image, width*height, (float)a, (float)b, (float)c);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    timee = (end.tv_sec - start.tv_sec) *1.0e3 + (end.tv_nsec - start.tv_nsec) *1.0e-6;
    avg = timee/10;
    printf ("greyscale Asssembler:          %f \n", avg);
    //greyscale C
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 10; i++){
        greyscale_abc_V0 (input_image, tmp, width, height, a, b, c);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    timee = (end.tv_sec - start.tv_sec) *1.0e3 + (end.tv_nsec - start.tv_nsec) *1.0e-6;
    avg = timee/10;
    printf ("greyscale C:                   %f \n", avg);
    //Sobel Filter Pure C
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 10; i++){
        sobelFilter_V1(tmp, width, height, result_image);
        }
    clock_gettime(CLOCK_MONOTONIC, &end);
    timee = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) *1.0e-9;
    avg = timee/10;
    printf ("Sobel Filter Pure C:           %f \n", avg);
    //Sobel Filter SIMD
    greyscale_abc_V0 (input_image, tmp, width, height, a, b, c); 
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 10; i++){
        sobelFilter_V0(tmp, width, height, result_image);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    timee = (end.tv_sec - start.tv_sec)  + (end.tv_nsec - start.tv_nsec) *1.0e-9;
    avg = timee/10;
    printf ("Sobel Filter SIMD:             %f \n", avg);
    //greyscale C + Sobel SIMD 
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 100; i++){
        greyscale_abc_V0 (input_image, tmp, width, height, a, b, c);
        sobelFilter_V0(tmp, width, height, result_image);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    timee = (end.tv_sec - start.tv_sec)  + (end.tv_nsec - start.tv_nsec) *1.0e-9;
    avg = timee/100;
    printf ("greyscale C + Sobel SIMD:      %f \n\n", avg);
    free(result_image);
    free(tmp);

}
//computes correct height and width for certain image sizes
void compare_complete (size_t complete, size_t* width, size_t* height){
   if (complete == 2500000){
    *width = 2000;
    *height = 1250;
   }
   if (complete == 2750000){
    *width = 2000;
    *height = 1375;
   }
   if (complete == 5500000){
    *width = 2750;
    *height = 2000;
   }
   if (complete == 5750000){
    *width = 2875;
    *height = 2000;
   }
   if (complete == 6250000){
    *width = 3125;
    *height = 2000;
   }
   if (complete == 6500000){
    *width = 3250;
    *height = 2000;
   }
   if (complete == 6750000){
    *width = 3375;
    *height = 2000;
   }
   if (complete == 7250000){
    *width = 3625;
    *height = 2000;
   }
   if (complete == 7750000){
    *width = 3875;
    *height = 2000;
   }
   if (complete == 8250000){
    *width = 4125;
    *height = 2000;
   }
   if (complete == 8500000){
    *width = 4250;
    *height = 2000;
   }
   if (complete == 8750000){
    *width = 4375;
    *height = 2000;
   }
}
int main (){
    //sqrt test
	test_sqrt();
    //necessary variables for greyscale and Sobel
    size_t complete_size;
    uint8_t* input_image = NULL;
    size_t helper_height = 0;
    //generate images from 0.25*10^9 to 10*10^9 pixels
    for (int i = 1; i <= 40; i++){
        complete_size = i * 250000;
        if (i%4 == 1){
            helper_height = helper_height + 500;
        }
        height = helper_height;
        width = complete_size/height;
        //compare wether width and height need to be adjusted
        compare_complete (complete_size, &width, &height);
        input_image = malloc (3*complete_size*sizeof(uint8_t));
        if(!input_image){
            fprintf(stderr, "Error allocating memory for input_image Array");
            exit(EXIT_FAILURE);
        }
        //generate random image
        generate(input_image, width, height);
        printf("image:     complete size:%zupx, ", complete_size);
        //main test
        test_greyscale_and_sobel(input_image, width, height);
        free(input_image);
    }
}

