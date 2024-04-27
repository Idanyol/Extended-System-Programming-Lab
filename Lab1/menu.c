#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*returns a new array (after allocating space for it), such that each value in the new array is the result of applying the function f on the corresponding character in the input array. */
char* map(char *array, int array_length, char (*f) (char)){
  char* mapped_array = (char*)(malloc(array_length*sizeof(char)));
  for (int i = 0 ; i < array_length ; i++) {
      mapped_array[i]=(*f)(array[i]);
  }
  return mapped_array;
}

/* Ignores c, reads and returns a character from stdin using fgetc. */
char my_get(char c) {
    return fgetc(stdin);
}

/* If c is a number between 0x20 and 0x7E, cprt prints the character of ASCII value c followed by a new line. Otherwise, cprt prints the dot ('.') character. After printing, cprt returns the value of c unchanged. */
char cprt(char c) {
    if(c >= 0x20 && c <= 0x7E) {
        printf("%c\n", c);
    }
    else
    {
        printf(".\n");
    }
    return c;
}

/* Gets a char c and returns its encrypted form by adding 1 to its value. If c is not between 0x20 and 0x7E it is returned unchanged */
char encrypt(char c) {
    if(c >= 0x20 && c <=0x7E) {
        return c + 1;
    }
    return c;
}

/* Gets a char c and returns its decrypted form by reducing 1 from its value. If c is not between 0x20 and 0x7E it is returned unchanged */
char decrypt(char c) {
    if(c >= 0x20 && c <=0x7E) {
        return c - 1;
    }
    return c;
}

/* xprt prints the value of c in a hexadecimal representation followed by a new line, and returns c unchanged. */
char xprt(char c) {
    printf("%x\n", c);
    return c;
}

struct fun_desc {
    char *name;
    char (*fun)(char);
};

int main(int argc, char **argv){
 
    char *carray = (char*)(malloc(5*sizeof(char)));
    carray[0] = '\0';
    struct fun_desc menu[] = {{"Get string", my_get}, {"Print string", cprt}, {"Encrypt", encrypt}, {"Decrypt", decrypt}, {"Print Hex",xprt}, {NULL, NULL}};
    int bound = sizeof(menu) / sizeof(menu[0]) - 1;

    while (1) {
        //print menu
        printf("%s", "Select operation from the following menu: (ctrl^D for exit)\n");
        for (int i = 0 ; i < bound && menu[i].name != NULL ; i++) {
            printf("%i) %s\n", i, menu[i].name);
        }
        printf("Option : ");
        
        char buffer[1024];
        char* pointer = fgets(buffer, sizeof(buffer), stdin);

        if (pointer == NULL) { 
            break;
        }

        int option = atoi(buffer);

        if (option < 0 || option > bound - 1) {
            printf("Not within bounds\n");
            break;    
        }
        else
        {
            printf("Within bounds\n");
            char* temp = carray;
            carray = map(carray, 5, menu[option].fun);
            free(temp);
            printf("DONE.\n\n");
        }
    }
    
    free(carray);
    return 0;
}
