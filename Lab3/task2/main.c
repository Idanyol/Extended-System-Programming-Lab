#include "util.h"
#include <stdio.h>
#define SYS_WRITE 4
#define STDOUT 1

#define SYS_OPEN 5
#define O_RDWR 2
#define SYS_SEEK 19
#define SEEK_SET 0
#define SHIRA_OFFSET 0x291

extern int system_call();
extern void infector();

int main (int argc , char* argv[], char* envp[]){
   
    char* fileName;
    char buff[8192];
    int fileDesc;
    int bytesNum;
    
    if (argc > 1) {
        
        if (strncmp(argv[1], "-a" , 2) == 0){
            fileName = &argv[1][2];
            system_call(SYS_WRITE, STDOUT, fileName, strlen(fileName));
            system_call(SYS_WRITE, STDOUT, ": VIRUS ATTACHED\n", 18);
            infector(fileName);
        }

        else {
            fileName = argv[1];
            fileDesc = system_call(SYS_OPEN, fileName, 0, 0644); /* 0 - READONLY */
            if (fileDesc < 0) {
                system_call(SYS_WRITE, STDOUT, "Error opening the file\n", strlen("Error opening the file\n"));
                system_call(1,0x55);
            }

            bytesNum = system_call(3, fileDesc, buff,8192); /* 3 - READ */

            if(bytesNum > 0) { 
                system_call(SYS_WRITE, STDOUT, buff, bytesNum);
                system_call(SYS_WRITE,STDOUT,"\n",1);
            }
            if(bytesNum == -1) {
                system_call(SYS_WRITE, STDOUT, "Error reading file\n", strlen("Error reading file\n"));
                system_call(1,0x55); 
            }
            system_call(6, fileDesc); /* 6 - CLOSE */

        }
       
             
    } 
   
    return 0;
}
