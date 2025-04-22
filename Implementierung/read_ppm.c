#include "sobel.h"
#include <ctype.h>


// interprets the header from the file f
size_t ppm_header(FILE* f, size_t* width, size_t* height, int* img_colors){

  int width_read, height_read , maxcolor; 
  unsigned int i;
  char modus[128]; // 128 is the maximum line size in the header of the file
  char line[128];
  int readed_data = 0;

  // iterates over the header lines
  while(fgets(line, 128, f) != NULL){   
    int comment_flag = 0; // whether there is a comment in the line or not 
    for(i = 0; i < strlen(line); i++){
      if(isgraph(line[i]) && (comment_flag == 0)){
        if((line[i] == '#') && (comment_flag == 0)){
          comment_flag = 1;
        }
      }
    }

    // reads the header data when the comment_flag is not set
    // the number of elements already been read is added to readed_data
    if(comment_flag == 0){
      if(readed_data == 0){
        readed_data += sscanf(line, "%2s %d %d %d", modus, &width_read, &height_read, &maxcolor); 
      }else if(readed_data == 1){
        readed_data += sscanf(line, "%d %d %d", &width_read, &height_read, &maxcolor); 
      }else if(readed_data == 2){
        readed_data += sscanf(line, "%d %d", &height_read, &maxcolor); 
      }else if(readed_data == 3){
        readed_data += sscanf(line, "%d", &maxcolor); 
      }
    }

    // when all header data has been read
    if(readed_data == 4){
      break;
    }
  }
  // error when less data is in the header then needed for a correct ppm file
  if(readed_data != 4){
    fprintf(stderr, "Error: header is not complete\n");
    exit(EXIT_FAILURE);
  }
  

  if(strcmp(modus, "P6") != 0){ // P6 indicates a raw ppm file -> pixel are representated by pure binary values
    fprintf(stderr, "Error: Input file not in PPM P6 format!\n");
    exit(EXIT_FAILURE);
  }
  if(maxcolor > UINT8_MAX){
    fprintf(stderr, "Error: max color value should be less than 256\n");
    exit(EXIT_FAILURE);
  }

  *width = width_read;
  *height = height_read;
  *img_colors = maxcolor;

  return  *width * *height; // returns the number of pixels in the image
}

// interprets the data from the file f when the file is P6
void ppm_data(FILE* f, uint8_t* image, size_t width, size_t height) {
  // reads the complete data all in one 
  if(fread(image, 3 * width, height, f) != height){ // error when less was read then intended 
    fprintf(stderr, "Error: the picture is smaller than the provided size in header\n");
    free(image);
    exit(EXIT_FAILURE);
  }
  // checks wether there is data that has not been read
  if(fread(image, 1, 1, f) != 0){
    fprintf(stderr, "Error: the picture is bigger than the provided size in header\n");
    free(image);
    exit(EXIT_FAILURE);
  }
}



//gets a file pointer from a ppm file and reads the data from it
uint8_t* format(FILE* file, size_t* width, size_t* height) {
    int maxcolor;
    size_t size = ppm_header(file, width, height, &maxcolor); //reads header from the file

    uint8_t* image = (uint8_t*) malloc(3 * size * sizeof(uint8_t)); // allocates image array
    if(!image) {
      fprintf(stderr, "Error: could not allocate image array in format() (line 165)");
    }
    // reads values of pixels from the file
    ppm_data(file, image, *width, *height); // in P6 format
   
    return image;
}