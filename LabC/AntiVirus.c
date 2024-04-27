#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/param.h>

typedef struct virus {
unsigned short SigSize;
char virusName[16];
unsigned char* sig;
} virus;

typedef struct link link;
struct link {
link *nextVirus;
virus *vir;
};

//Global variebles
FILE* sigFile;
bool isBigEndian = false;
link* virusList;

unsigned short flipBytes(unsigned short num) {
    // Extract the two bytes
    unsigned char byte1 = num & 0xFF; // Least significant byte
    unsigned char byte2 = (num >> 8) & 0xFF; // Most significant byte

    // Swap the bytes
    unsigned short flippedNum = (byte1 << 8) | byte2;

    return flippedNum;
}


virus* readVirus(FILE* file) {
    struct virus *v = (virus *)malloc(sizeof(virus));
    
    int n1 = fread(v, 1, 18, file);
    if (n1 != 18) {
        free(v);
        return NULL;
    }

    if (isBigEndian) {
        v->SigSize = flipBytes(v->SigSize);
    }

    v->sig = (unsigned char*)malloc(sizeof(char) * v->SigSize);
    //printf("%hu", v->SigSize);
    int n2 = fread(v->sig, 1, v->SigSize, file);
    if (n2 != v->SigSize) {
        free(v->sig);
        free(v);
        return NULL;
    }

    return v;
}

void PrintHex (char buffer[], int length){
    int counter = 0;
    for(int i = 0; i < length; i++){
        counter++;
        printf ("%02X ", (unsigned char)buffer[i]);
        if (counter == 20) { //to be order
            printf("\n");
            counter = 0;
        }
    }
}

// this function receives a virus and a pointer to an output file. The function prints the virus to the given output. 
void printVirus(virus* virus, FILE* output){
  //print the virus name (in ASCII)
  printf("Virus name: %s\n", virus->virusName);
  
  // print the virus signature length (in decimal)
  printf("Virus size: %hu\n", virus->SigSize);

  //print the virus signature (in hexadecimal representation). 
  printf("Signature: \n");
  PrintHex((char*)virus->sig, virus->SigSize);
  printf("\n");
}

void ass1a () {
    FILE *inputB = fopen("signatures-B", "r");
    char buff[5];
    fread(buff, 1, 4, inputB);
    buff[4]='\0';
    if (strcmp(buff, "VIRB") != 0) {
        printf("Error");
        //exit(1);
    }
    else {
        isBigEndian = true;
        virus *v;
        while ((v = readVirus(inputB)) != NULL) {
            printVirus(v, stdout);
            printf("\n");
            free(v->sig);
            free(v);
        }
    }

    FILE *inputL = fopen("signatures-L", "r");
    char buff2[5];
    fread(buff2, 1, 4, inputL);
    buff2[4]='\0';
    if (strcmp(buff2, "VIRL") != 0) {
        printf("Error");
        //exit(1);
    }
    else {
        virus *v;
        while ((v = readVirus(inputL)) != NULL) {
            printVirus(v, stdout);
            printf("\n");
            free(v->sig);
            free(v);
        }
    }
}


/* Print the data of every link in list to the given stream. Each item followed by a newline character. */
void list_print(link *virus_list, FILE* file) {
    while (virus_list != NULL) {
        printVirus(virus_list->vir, file);
        virus_list = virus_list->nextVirus;
    }
}

/* Add a new link with the given data to the list (at the end CAN ALSO AT BEGINNING), and return a pointer to the list (i.e., the first link in the list). If the list is null - create a new entry and return a pointer to the entry. */
link* list_append(link* virus_list, virus* data) {
    link *l = malloc(sizeof(link));
    l->nextVirus = virus_list;
    l->vir = data;
    return l;
}

/* Free the memory allocated by the list. */
void list_free(link *virus_list) {
    link *l = virus_list;
    while (l != NULL) {
        virus *v = l->vir;
        free(v->sig);
        free(v);
        link *temp = l;
        l=l->nextVirus;
        free(temp);
    }
}

struct fun_desc {
    char *name;
    void (*fun)();
};

void loadSig() {
    if(virusList != NULL){
        list_free(virusList); 
    }
    char input[1024];
    printf("Enter file name: ");

    if(fgets(input, 1024, stdin) == NULL) {
        printf ("Error loading the file\n");
        //exit(1);
    }

    input[strcspn(input, "\n")] = '\0';
    sigFile = fopen(input, "r");
    
    if (sigFile == NULL) {
        printf ("Error loading the file\n");
       // exit(1);
    }

    char buff[5];
    fread(buff, 1, 4, sigFile);
    buff[4]='\0';
    if (strcmp(buff, "VIRB") != 0 && strcmp(buff, "VIRL") != 0) {
        printf("Error loading the file\n");
        if(virusList != NULL){
            list_free(virusList); 
        }
        exit(1);
    }
    if (strcmp(buff, "VIRB") == 0) {
        isBigEndian = true;
    }
    //build the link list of virus --
    virus *v = readVirus(sigFile);
    virusList = NULL;
    while (v != NULL) {
        virusList = list_append(virusList, v);
        v=readVirus(sigFile);
    }  
    fclose(sigFile); 
    
}

void printSig () {
    if (sigFile == NULL) {
        printf("No file was detected\n");
    }
    else {
        list_print(virusList, stdout);     
    }    
}

void quit() {
    if(virusList != NULL){
        list_free(virusList); 
    }
    exit(0);
}

void detect_virus(char *buffer, unsigned int size, link *virus_list) {
    for (int i = 0 ; i < size ; i++) {
        link *l=virus_list;
        while (l != NULL) {
            virus *v = l->vir;
            int len = v->SigSize;
            if (memcmp(buffer + i, v->sig, len) == 0) {
                printf("The starting byte location in the suspected file: %d \n", i);
                printf("The virus name: %s \n", v->virusName);
                printf("The size of the virus signature: %d \n\n", v->SigSize);
            }
            l=l->nextVirus;
        }
    }
    printf("\n");
}

void detectViruses() {
    printf("Enter infected file: \n");
    char input[512];
    
    if (fgets(input, 512, stdin) == NULL) {
        printf ("Error loading the file\n");
        //exit(1);
    }

    input[strcspn(input, "\n")] = '\0';
    FILE *file;
    if ((file = fopen(input, "r")) == NULL) {
        printf ("No such file\n");
        return;
    }
    char buffer[10000];
    size_t minSize = fread(buffer, 1, 10000, file); //read the file into the buffer and return the number of bytes that success to read
    if ( minSize == 0) {
        printf("Error reading the file");
        return;
    }
    else {
        detect_virus(buffer, minSize, virusList);
    }
    fclose(file); 
}

void neutralize_virus(char *fileName, int signatureOffset){
    char ret[] = {0xC3};
    FILE *file = fopen(fileName, "r+");
    fseek(file, signatureOffset, SEEK_SET);
    fwrite(ret, 1, 1, file);
    fclose(file);
}

void fixFile() {
    printf("Enter file to fix: \n");
    char input[512];
    
    if (fgets(input, 512, stdin) == NULL) {
        printf ("Error loading the file\n");
        //exit(1);
    }

    input[strcspn(input, "\n")] = '\0';
    FILE *file;
    if ((file = fopen(input, "r")) == NULL) {
        printf ("No such file\n");
        return;
    }
    else {
        char buffer[10000];
        size_t minSize = fread(buffer, 1, 10000, file); //read the file into the buffer and return the number of bytes that success to read
        if (minSize == 0) {
            printf("Error reading the file");
            return;
        }
        else {
            for (int i = 0 ; i < minSize ; i++) {
                link *l=virusList;
                while (l != NULL) {
                    virus *v = l->vir;
                    int len = v->SigSize;
                    if (memcmp(buffer + i, v->sig, len) == 0) {
                        neutralize_virus(input, i);
                    }
                    l=l->nextVirus;
                }        
            }
        }
    }
    fclose(file);
}

int main(int argc, char **argv) {
    
    /* building menu */
    struct fun_desc menu[] = {{"Load signatures", loadSig}, {"Print signatures", printSig}, {"Detect viruses", detectViruses}, {"Fix file", fixFile}, {"Quit",quit}, {NULL, NULL}};
    int bound = sizeof(menu) / sizeof(menu[0]) - 1;

    while (1) {
        //print menu
        printf( "Select operation from the following menu: (type quit for exit)\n");
        for (int i = 0 ; i < bound && menu[i].name != NULL ; i++) {
            printf("%i) %s\n", i+1, menu[i].name);
        }
        printf("Option : ");
        
        char buffer[1024];
        char* pointer = fgets(buffer, sizeof(buffer), stdin);

        if (pointer == NULL) { 
            break;
        }

        int option = atoi(buffer);

        if (option < 1 || option > bound) {
            printf("Not within bounds\n");
            break;    
        }
        else {
            menu[option-1].fun();
        }
    }
    list_free(virusList); 
    
    return 0;
}



