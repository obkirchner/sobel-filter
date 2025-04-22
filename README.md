# Description

### Usage:           `./sobel [options] input_file`


A given image in the form of a PPM-file gets converted into grayscale and then the edges get outlined by applying the Sobel filter




|Options                        |Description                                         |
|-------------------------------|-----------------------------                        |
|`-V <version>`                 |Define which version should be used, there exist 2 versions: V0, them main version and V1, which uses a different, slower implementation                              
|`-B [cycles]`                  |Print out run-time of the the main-function, if [cycles] is given, it defines how often it is called 
|`--coeffs <r>,<g>,<b>`         |Coefficients of red, green and blue respectively of the weighted grayscale conversion, if not set, then 1.0,1.0,1.0 by default.                 |
|`-o <output_file>`             |Define in which file the result should be saved, if not set, then the output file is named "sobel_filtered_\*inputfile name\*.pgm"                                                                                 |
|`-h`                           |show this descritpion as help message         |
|`--help`                       |show this descritpion as help message         |

### Example
`./sobel -V0 -B1000 --coeffs 21.25,71.54,7.21 example.ppm -o result.pgm`

This programm call converts the content of example.ppm into grayscale using the coefficients 21.25 for red, 71.54 for green and 7.21 for blue. It then applies the sobel filter to it using the main version V0 and saves the result into result.pgm. Apart from that it shows the runtime of 1000 function calls.