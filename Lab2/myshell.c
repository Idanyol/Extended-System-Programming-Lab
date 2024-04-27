#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <unistd.h>
#include "LineParser.c"
#include <wait.h>
#include <fcntl.h>
#include <signal.h>

int debug = 0; 
void execute (cmdLine *pCmdLine){
    int succ; //if get error: print in the end of the function   
    if(strcmp(pCmdLine->arguments[0], "cd") == 0)
        succ = chdir(pCmdLine->arguments[1]);

    else if(strcmp(pCmdLine->arguments[0], "wakeup") == 0)
        succ = kill (atoi(pCmdLine->arguments[1]), SIGCONT);

    else if(strcmp(pCmdLine->arguments[0], "nuke") == 0)
        succ = kill (atoi(pCmdLine->arguments[1]), SIGINT);
   
    else{
        pid_t pid;
        int stat;
        pid = fork();
        if(pid < 0){ 
            perror("fork fail");
        }
        else if (pid == 0){ 
            //in the child process
            //Redirection input/output
            FILE* inputStream;
            FILE* outputStream;
            if (pCmdLine->inputRedirect != NULL){    
                inputStream = fopen(pCmdLine->inputRedirect,"r");
                if(inputStream == NULL)
                    perror("redirection input stream is failed!");
                else{
                    dup2(fileno(inputStream),STDIN_FILENO);
                }
                fclose(inputStream);
            }
            if (pCmdLine->outputRedirect != NULL){
                outputStream = fopen(pCmdLine->outputRedirect,"w");
                if(outputStream == NULL)
                    perror("redirection output stream is failed!");
                else{
                    dup2(fileno(outputStream),STDOUT_FILENO);
                }
                fclose(outputStream);
            }

            stat = execvp(pCmdLine->arguments[0],pCmdLine->arguments);
            perror("execv failed!");
            freeCmdLines(pCmdLine);
            exit(1);
        }
        else{
            //in the parent process
            if(pCmdLine->blocking == 1){
                waitpid(pid,&stat,0); 
            }
            if (debug == 1)
                fprintf(stderr, "PID: %d, executing: %s\n",pid, pCmdLine->arguments[0]);
            freeCmdLines(pCmdLine);
        }
    }
    if (succ == -1)
        perror("operation fail!" );
}


int main(int argc, char **argv){
    //check debug flag
    for (int i = 1; i < argc; i++){ 
        if(strcmp(argv[i],"-d") == 0){
            debug = 1; 
            break;
        }
    }
    while (1)
    {
        char path[PATH_MAX], userInput[2048];  
        //printing path
        if(getcwd(path, sizeof(path)) == NULL){
            perror("getcwd() return an error!");
            exit(1);
        }
        else
            printf ("%s\n", path);

        // get input from keyboard (user)
        cmdLine *inputAfterParse;
        if(fgets(userInput, 2048, stdin) == NULL){
            perror("fgets() return an error" );
        }
        else{ 
            //parse the input
            inputAfterParse = parseCmdLines(userInput);  
            if(inputAfterParse != NULL){
                if(strcmp(inputAfterParse->arguments[0],"quit") == 0){
                    freeCmdLines(inputAfterParse);
                    exit(0);
                }

                execute(inputAfterParse);
            }
        }
    }
    return 0; 
}
