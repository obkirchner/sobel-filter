
CFLAGS=-std=gnu99 -O3 -Wall -Wextra -Wpedantic -g -msse4.1

all: sobel
sobel: main.c sobel.h sqrtFunctions.h ISqrtFunctions.c sobelFilterPureC.c sobelIntrinsics.c read_ppm.c write_pgm.c toGrayscalePureC.c sobelFunctions.c greyscaleASM.S
	$(CC) $^ $(CFLAGS) -o $@ 
bench: benchmarking.c  generate_images.c sobel.h sqrtFunctions.h ISqrtFunctions.c sobelFilterPureC.c sobelIntrinsics.c read_ppm.c write_pgm.c toGrayscalePureC.c sobelFunctions.c greyscaleASM.S 
	$(CC) $^ $(CFLAGS) -lm -o $@ 
compare: comparePGM.c generate_images.c sobel.h sqrtFunctions.h ISqrtFunctions.c sobelFilterPureC.c sobelIntrinsics.c read_ppm.c write_pgm.c toGrayscalePureC.c sobelFunctions.c greyscaleASM.S 
	$(CC) $^ $(CFLAGS) -lm -o $@ 
clean:
	rm -f sobel
