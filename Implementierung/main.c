#define _POSIX_C_SOURCE 199309L
#include "sobel.h"
#include <stddef.h>
#include <getopt.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <libgen.h>

void print_usage(){
    fprintf (stderr, "\nUsage:                   ./sobel [-V <version>] [-B [cycles]] [-h] [--help] [--coeffs <FP number>,<FP number>,<FP number>] input_file -o <output_file> \n\n"
                     "For more information :   './sobel -h' or './sobel --help'\n\n");
}

int main (int argc, char* argv[]){

    //declaring variables
    int opt = 0;
    long version = 0;
    int B_flag = 0;
    long B_number = 1;
    int o_flag = 0;
    char* outputfile_string = NULL;
    int h_flag = 0;
    errno = 0;
    char* endptr;
    char* coeffs;
    float a = 1;
    float b = 1; 
    float c = 1;
    FILE *inputfile = NULL;
    FILE *outputfile = NULL;
    int failure = 0;
    uint8_t* inputImg;
    uint8_t* resultImg;
    size_t height = 0;
    size_t width = 0;
    struct timespec start, end;
    double time = 0.0;

    

    //necessary for getopt_long
    static const struct option longopts[] = 
        {
            {"help", no_argument, NULL, 'h'},
            {"coeffs", required_argument, 0, 0},
            {0, 0, 0, 0}
        };
    
    //parse command line arguments
    while ((opt = getopt_long (argc, argv, "V:B::o:h", longopts, NULL)) != -1){
        switch (opt){
            case 'V':
                if (optarg == NULL){
                    fprintf (stderr, "\n\n-V needs the version number as argument\n");
                    print_usage();
                    return EXIT_FAILURE;
                } else {
                    version = strtol (optarg, &endptr, 10);
                    if(endptr == optarg || *endptr != '\0' || errno == ERANGE){
                        fprintf (stderr , "\n\ninvalid version number \n");
                        print_usage();
                        return EXIT_FAILURE;
                    }
                    if (version != 0 && version != 1){
                        fprintf(stderr, "\n\nthere are only two versions of sobel. Choose one of them");
                        print_usage();
                        return EXIT_FAILURE;
                    }
                }
                break;
            case 'B':
                B_flag = 1;
                if (optarg == NULL){
                    break;
                } else  {
                    B_number = strtol (optarg, &endptr, 10);
                    if(endptr == optarg || *endptr != '\0' || errno == ERANGE){
                        fprintf (stderr , "\n\ninvalid B argument \n") ;
                        print_usage();
                        return EXIT_FAILURE;
		            }
                }
                break;
            case 'o':
                if (optarg == NULL){
                    fprintf (stderr, "\n\n-o needs an output file as argument\n");
                    print_usage();
                    return EXIT_FAILURE;
                }
                if (strlen (optarg) <= 255){
                    int outputfile_string_length = strlen(optarg);
                    outputfile_string = malloc(outputfile_string_length);
                    if (outputfile_string == NULL){
                        fprintf (stderr, "\nmemory allocation setting the output file failed\n");
                        return EXIT_FAILURE;
                    }
                    strcpy(outputfile_string,optarg);
                } else {
                    fprintf (stderr, "name of outputfile must have a max length of 255 characters");
                }
                o_flag = 1;
                break;
            case 'h':
                h_flag = 1;
                goto print_help_message;
                break;
            //https://stackoverflow.com/questions/15822660/how-to-parse-a-string-separated-by-commas
            case 0:
                if (optarg == NULL){
                    fprintf (stderr, "\n\n-coeffs needs the coefficient a,b,c as argument\n");
                    print_usage();
                }
                coeffs = strtok (optarg,",");
                if (coeffs == NULL){
                    goto a_error;
                }
                a = strtof (coeffs, &endptr);
                if(endptr == coeffs){
                    a_error:
                    fprintf (stderr , "\n\ninvalid coefficient a\n");
                    print_usage();
                    free(outputfile_string);
                    return EXIT_FAILURE;
                }
                coeffs = strtok (NULL,",");
                 if (coeffs == NULL){
                    goto b_error;
                }
                b = strtof (coeffs, &endptr);
                if(endptr == coeffs){
                    b_error:
                    fprintf (stderr , "\n\ninvalid coefficient b\n");
                    print_usage();
                    free(outputfile_string);
                    return EXIT_FAILURE;
                }
                coeffs = strtok (NULL,",");
                 if (coeffs == NULL){
                    goto c_error;
                }
                c = strtof (coeffs, &endptr);
                if(endptr == coeffs){
                    c_error:
                    fprintf (stderr , "\n\ninvalid coefficient c\n");
                    print_usage();
                    free(outputfile_string);
                    return EXIT_FAILURE;
                }
                break;
            default: 
                print_usage();
                free(outputfile_string);
                return EXIT_FAILURE;
        }
    }

    //check whether positional argument input file exists 
    if (optind >= argc){
        fprintf (stderr, "\n\nmissing input file\n");
        print_usage();
        free(outputfile_string);
        return EXIT_FAILURE;
    }
    
    //open input file
    if (!(inputfile = fopen (argv[optind], "rb"))){
        fprintf (stderr, "\nError opening input file\n");
        print_usage();
        free(outputfile_string);
        return EXIT_FAILURE;
    }
    struct stat statbuf;
    if (fstat (fileno(inputfile), &statbuf)){
        fprintf (stderr, "\nError retrieving file stats\n");
        failure = 1;
        goto cleanup;
    }
    if (!S_ISREG(statbuf.st_mode) || statbuf.st_size <= 0){
        fprintf (stderr, "\nError processing file: Not a regular file or invalid size\n");
        failure = 1;
        goto cleanup;
    }


    //create new output file name by calling it "sobel_filtered_*inputfile name*.pgm", if -o isn't set
    // https://stackoverflow.com/questions/2736753/how-to-remove-extension-from-file-name
    if (o_flag == 0){
        char* helper = basename (argv[optind]);
        char* last_point;
        int name_length = 0;
        last_point = strrchr (helper, '.');
        if (last_point != NULL){
            *last_point = '\0';
        }
        name_length = strlen(helper);
        outputfile_string = malloc(name_length+30);
        if (!outputfile_string){
            fprintf (stderr, "\nmemory allocation naming the output file failed\n");
            return EXIT_FAILURE;
        }
        strcat(outputfile_string, "sobel_filtered_");
        strcat(outputfile_string, helper);
        strcat(outputfile_string, ".pgm");
    } 
    //create output file
    if (!(outputfile = fopen (outputfile_string, "w"))){
        fprintf (stderr, "\nerror opening output file\n");
        free(outputfile_string);
        return EXIT_FAILURE;
    }

    print_help_message:
    if (h_flag == 1){
        print_usage();
        printf("\nHelp Message:\n\n"
            "\nUsage:                   ./sobel [-V <version>] [-B [cycles]] [--coeffs <FP number>,<FP number>,<FP number>] [-h] [--help] input_file [-o <output_file>] \n\n"
               "Description:  A given image in the form of a PPM-file gets converted into grayscale and then the edges get outlined by\n" 
               "              applying the Sobel filter\n"
               "Options:      -V <version>               Define which version should be used, there exist 2 versions: V0, them main version and V1, which uses a different, slower implementation\n"
               "              -B [cycles]                Print out run-time of the the main-function, if [cycles] is given, it defines how often it is called\n"
               "              --coeffs <a>,<b>,<c>       Coefficients of red, green and blue respectively of the weighted grayscale conversion, if not set, then 1.0, 1.0, 1.0 by default.\n"
               "                                         Needs to be beetwen 1.0 and 255.0. There must not be spaces inbetween.\n"
               "              -h                         Show this help message\n"
               "              --help                     Show this help message\n"
               "              -o <output_file>           Define in which file the result should be saved, if not set, then the output file is named: \"sobel_filtered_*inputfile name*.pgm\"\n"
               "Example:       ./sobel -V0 -B1000 --coeffs 21.25,71.54,7.21 example.ppm -o result.pgm\n"
               "               This programm call converts the content of example.ppm into grayscale using the coefficients 21.25 for red, 71.54 for green and 7.21 for blue and then\n"
               "               applies the sobel filter to it using the main version V0 and saves the result in result.pgm.\n"
               "               Apart from that it shows the runtime of 1000 function calls.\n\n"
        );
        return EXIT_SUCCESS;
    }
    //read in file
    inputImg = format(inputfile, &width, &height);

    void* tmp = malloc(height*width*(sizeof(uint8_t)));
    if(!tmp){
        free(inputImg);
        free(outputfile_string);
        fprintf(stderr, "Error allocating memory for tmp Array in main.c (l. 256)");
        exit(EXIT_FAILURE);
    }

    resultImg = malloc(height*width*(sizeof(uint8_t)));
    if(!resultImg){
        free(inputImg);
        free(tmp);
        free(outputfile_string);
        fprintf(stderr, "Error allocating memory for resultImg Array in main.c (l. 263)");
        exit(EXIT_FAILURE);
    }

    if (version == 0){
        clock_gettime(CLOCK_MONOTONIC, &start);
        for (int i = 0; i < B_number; i++){
            //edge case 
            if(width < 5 || height < 5){
                sobel_V1(inputImg, width, height, a, b, c, tmp, resultImg);
            }else{
                sobel(inputImg, width, height, a, b, c, tmp, resultImg);
            }
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        time = (end.tv_sec - start.tv_sec) * 1.0e3 + (end.tv_nsec - start.tv_nsec)* 1.0e-6;
    }

    else if (version == 1){
        clock_gettime(CLOCK_MONOTONIC, &start);
        for (int i = 0; i < B_number; i++){
            sobel_V1(inputImg, width, height, a, b, c, tmp, resultImg);
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        time = (end.tv_sec - start.tv_sec) * 1.0e3 + (end.tv_nsec - start.tv_nsec) *1.0e-6;    
    }
    
    writePGM(outputfile, resultImg, width, height);
  
    //if -B is set print execution time
    if (B_flag == 1){
        printf("\n\nExecution Time of %ld function calls: %.9fms\n\n", B_number, time); 
    }
    
    cleanup:
        if(inputfile){
            fclose(inputfile);
        }
        if(outputfile){
            fclose(outputfile);
        }
        if (failure  == 1){
            return EXIT_FAILURE;
        } 
        free (outputfile_string);
        free (tmp);
        free (inputImg);
        free (resultImg);

    return EXIT_SUCCESS;
}
