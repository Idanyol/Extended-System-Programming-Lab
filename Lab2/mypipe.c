#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>



int main(int argc, char **argv) {
    char msg[512]; 
    int pipeArray[2]; 
    pid_t pid;
    
    if (pipe(pipeArray) == -1) {
        perror("Building pipe failed!");
        exit(1);
    }

    if ((pid = fork()) == -1) {
        perror("fork");
        exit(1);
    } 
    else if (pid == 0) { // Child process
        close(pipeArray[0]); 
        // Send message to parent process
        char *msg = "hello";
        write(pipeArray[1], msg, 512);
        close(pipeArray[1]); 
        exit(0);
    } 
    else { // Parent process
        close(pipeArray[1]);
        //read the msg 
        read(pipeArray[0], msg,512);
        printf("Child's msg: %s\n", msg);
        close(pipeArray[0]); 
        exit(1);
    }

    return 0;
}