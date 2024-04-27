#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

void encode(FILE *infile, FILE *outfile, int sign, const char *key);

int main(int argc, char *argv[]) {
    FILE *infile = stdin, *outfile = stdout;
    bool debugMode=true;
    const char *key;
    int sign;

    for (int i = 1 ; i < argc ; i++) {
        
        //print argv if debugMode is on
        if (debugMode) {
            printf("%s\n", argv[i]);
        }

        //loop over argv
        if (argv[i][0] == '+') {
            if (argv[i][1] == 'D') {
                debugMode=true;
            } 
            else if(argv[i][1] == 'E') {
                key=argv[i] + 2;
                sign=1;
            }
        }
        else if (argv[i][0] == '-') {
            if (argv[i][1] == 'D') {
                debugMode=false;
            } 
            else if (argv[i][1] == 'E') {
                key=argv[i] + 2;
                sign=-1;
            }
            else if (argv[i][1] == 'I') {
                infile = fopen(argv[i]+2,"r");
                if (infile == NULL) {
                    fprintf(stderr, "Failed to open the input file\n");
                    exit(1);
                }
            } 
            else if (argv[i][1] == 'O') {
                outfile = fopen(argv[i]+2, "w");
                if (infile == NULL) {
                    fprintf(stderr, "Failed to open the output file\n");
                    exit(1);
                }
            }                
        }
    }

    encode(infile, outfile, sign, key);
    fclose(infile);
    fclose(outfile);
    return 0;
}

void encode(FILE *infile, FILE *outfile, int sign, const char *key) {
    int i=0;
    int ch;
    // Read characters from input file and encode
    while ((ch = fgetc(infile)) != EOF) {
        
        if (key[i] == '\0') {
            i=0;
        }
        int digit = key[i] - '0';
        i++;

        if (ch > 47 && ch < 58) {
            ch = (ch + digit*sign - '0' + 10) % 10 + '0';
        }
        else if (ch > 64 && ch < 91) {
            ch = (ch + digit*sign - 'A' + 26) % 26 + 'A';
        }
             
        // Write the encoded character to the output file
        fputc(ch, outfile);
    }
}
