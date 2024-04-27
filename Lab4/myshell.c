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

#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0
#define HISTLEN 20

typedef struct process{
    cmdLine* cmd;                         /* the parsed command line*/
    pid_t pid; 		                  /* the process id that is running the command*/
    int status;                           /* status of the process: RUNNING/SUSPENDED/TERMINATED */
    struct process *next;	                  /* next process in chain */
} process;
    

int debug = 0; 

char *cHistory[HISTLEN];
int counter = 0; //counts the commands
int oldest = 0;

void inputRedirect(cmdLine *pCmdLine) {
    FILE *inputStream = fopen(pCmdLine->inputRedirect,"r");
    if(inputStream == NULL)
        perror("redirection input stream is failed!");
    else{
        dup2(fileno(inputStream),STDIN_FILENO);
    }
    fclose(inputStream);
}

void outputRedirect(cmdLine *pCmdLine) {
    FILE *outputStream = fopen(pCmdLine->outputRedirect,"w");
    if(outputStream == NULL)
        perror("redirection output stream is failed!");
    else{
        dup2(fileno(outputStream),STDOUT_FILENO);
    }
    fclose(outputStream);
}

void myPipe(cmdLine *pCmdLine) {
    int pipeArray[2]; 
    
    if (pipe(pipeArray) == -1) {
        perror("Building pipe failed!\n");
        exit(1);
    }

    pid_t pid_firstChild;
    if ((pid_firstChild = fork()) == -1) {
        perror("fork");
        exit(1);
    } 
    else if (pid_firstChild == 0) { 
        // Child1 process
        if (pCmdLine->inputRedirect != NULL) {
            inputRedirect(pCmdLine);
        }
        if (pCmdLine->outputRedirect != NULL) {
            perror("Error");
        }
        close(STDOUT_FILENO);
        dup(pipeArray[1]);
        close(pipeArray[1]);
        execvp(pCmdLine->arguments[0],pCmdLine->arguments);
    }
    else { 
        // Parent process
        int stat1;
        close(pipeArray[1]);
        
        pid_t pid_secChild;

        if ((pid_secChild = fork()) == -1) {
            perror("fork");
            exit(1);
        } 

        else if (pid_secChild == 0) { 
            // Child2 process
            if (pCmdLine->next->inputRedirect != NULL) {
                perror("Error");
            }
            if (pCmdLine->next->outputRedirect != NULL) {
                outputRedirect(pCmdLine->next);
            }
            close(STDIN_FILENO);
            dup(pipeArray[0]);
            close(pipeArray[0]);
            execvp(pCmdLine->next->arguments[0],pCmdLine->next->arguments);
        }

        else { 
            // Parent process
            int stat2;
            close(pipeArray[0]);
            waitpid(pid_firstChild,&stat1,0);
            waitpid(pid_secChild,&stat2,0); 
        } 
    }

}

void addProcess(process** process_list, cmdLine* cmd, pid_t pid){
    process* new_process = (process *)malloc(sizeof(process));
    new_process->cmd = cmd; 
    new_process->pid = pid;
    new_process->status = 1; 
    new_process->next = *process_list;
    *process_list = new_process;
    // if (process_list == NULL) {
    //     *process_list = new_process;
    //     (*process_list)->next = NULL;
    // }
    // else
    //     new_process->next = *process_list;
    // *process_list = new_process; 
}

/* go over the process list, and for each process check if it is done. */
void updateProcessList(process **process_list) {
    process *curr = *process_list;
    while(curr != NULL){
        int stat;
        int inProc = waitpid((curr)->pid, &stat, WNOHANG);
        if(inProc == -1) 
            printf("Error with process id");
        else if (inProc != 0 ){ //the child terminated
            (curr)->status = TERMINATED; 
        }
        curr = curr->next;
    }
}

void printProcessList(process** process_list) {
    updateProcessList(process_list);
    process *prev = NULL;
    process *curr = *process_list;
    int ind = 0;
    printf("Index\tPID\tCommand\tSTATUS\n");
    while (curr != NULL) {
        ind++; //starts from 1, if should start from 0 should be moved down
        pid_t pid = curr->pid;
        int stat = curr->status;
        char *status;
        switch (stat) {
            case TERMINATED:
                status="TERMINATED";
                break;
            case RUNNING:
                status="RUNNING";
                break;
            case SUSPENDED:
                status="SUSPENDED";
                break;
        }
        cmdLine *c = curr->cmd;
        printf("%d\t%d\t%s\t%s\n", ind, pid, status, c->arguments[0]);
        // if terminated, delete the node from the list
        if (stat == -1) {    
            process *temp = curr;
            if (prev == NULL) {
                *process_list = curr->next;
            }
            else {
                prev->next = curr->next;
            }
            curr = curr->next;
            freeCmdLines(temp->cmd);
            free(temp);
        }
        else {
            prev = curr;
            curr = curr->next;
        }    
    }
}

/* find the process with the given id in the process_list and change its status to the received status. */
void updateProcessStatus(process* process_list, int pid, int status) {
    process *curr = process_list;
    while (curr != NULL && curr->pid != pid) {
        curr = curr->next;
    }
    if(curr == NULL) {
        printf("process not found");
    }
    else {
        curr->status = status;
    }
}

void freeProcessList(process* process_list){
    while(process_list != NULL){
        freeCmdLines(process_list->cmd);
        process* temp_proc = process_list;
        process_list = process_list->next;
        free(temp_proc);
    }
}
/* inv: index will point to the next free cell*/
/* inv2: oldest will point to the oldest command line*/
void addHistory(char* userInput) {
    char* comm = (char*)malloc(sizeof(char)*sizeof(userInput));
    strcpy(comm,userInput);
    if (counter >= HISTLEN) { //arr is full
        free(cHistory[oldest]);
        cHistory[oldest] = comm;
        oldest = (oldest + 1) % HISTLEN;
        counter++;
    }
    else { //arr not full
        cHistory[counter] = comm;
        counter++;
    }   
}

void freeHistory() {
    for (int i=0 ; i < HISTLEN ; i++) {
        free(cHistory[i]);
    }
}

void printHistory() {
    printf("%d\t%s", oldest, cHistory[oldest]);
    for (int i=(oldest + 1) % HISTLEN ; i != oldest ; i = (i+1) % HISTLEN) {
        if(cHistory[i] != NULL) {
            printf("%d\t%s", i, cHistory[i]);
        }  
    }
}

void execute (cmdLine *pCmdLine, process** process_list){
    int succ; //if get error: print in the end of the function   
    if (pCmdLine->next != NULL) {
        myPipe(pCmdLine);
    }
    else if (strcmp(pCmdLine->arguments[0], "cd") == 0) {
        succ = chdir(pCmdLine->arguments[1]);
        freeCmdLines(pCmdLine);
    }
    else if(strcmp(pCmdLine->arguments[0], "wakeup") == 0) {
        succ = kill(atoi(pCmdLine->arguments[1]), SIGCONT);
        updateProcessStatus(*process_list, atoi(pCmdLine->arguments[1]), 1);
        freeCmdLines(pCmdLine);
    }
    else if(strcmp(pCmdLine->arguments[0], "nuke") == 0) {
        succ = kill(atoi(pCmdLine->arguments[1]), SIGINT);
        updateProcessStatus(*process_list, atoi(pCmdLine->arguments[1]), -1);
        freeCmdLines(pCmdLine);
    }
    else if(strcmp(pCmdLine->arguments[0], "suspend") == 0) {
        succ = kill(atoi(pCmdLine->arguments[1]), SIGTSTP);
        updateProcessStatus(*process_list, atoi(pCmdLine->arguments[1]), 0);
        freeCmdLines(pCmdLine);
    }
    else if(strcmp(pCmdLine->arguments[0], "procs") == 0) {
        printProcessList(process_list);
        freeCmdLines(pCmdLine);
    }
    else if(strcmp(pCmdLine->arguments[0], "history") == 0) {
        printHistory();
        freeCmdLines(pCmdLine);
    }
    else if(strcmp(pCmdLine->arguments[0], "!!") == 0) {
        cmdLine* old = parseCmdLines(cHistory[(counter-1) % HISTLEN]);
        addHistory(cHistory[(counter-1) % HISTLEN]);  //HERE!!
        execute(old,process_list);
        freeCmdLines(pCmdLine); //if succeeded wont get here and then how will free the cmdLine???
    }
    /* add "=" to the atoi >= 0*/
    else if((pCmdLine->arguments[0][0]) == '!' && isdigit((pCmdLine->arguments[0])[1]) != 0 && atoi(pCmdLine->arguments[0]+1) >= 0 && atoi(pCmdLine->arguments[0]+1) <= HISTLEN) {
    //else if(strcmp(strncpy(pCmdLine->arguments[0],0,1), "!") == 0 && atoi((pCmdLine->arguments[0])[1])-1 > 0 && atoi((pCmdLine->arguments[0])[1])-1 <= HISTLEN) {
        if (cHistory[atoi(pCmdLine->arguments[0]+1)] != NULL) {
            cmdLine* old = parseCmdLines(cHistory[atoi(pCmdLine->arguments[0]+1)]);
            addHistory(cHistory[atoi((pCmdLine->arguments[0])+1)]);
            execute(old,process_list);
            freeCmdLines(pCmdLine);
        }
        else {
            printf("Index does not exist\n");
        }
    }
    //rest of commands
    else {
        pid_t pid;
        int stat;
        pid = fork();
        if(pid < 0){ 
            perror("fork fail");
        }
        else if (pid == 0){ 
            //in the child process
            //Redirection input/output
            if (pCmdLine->inputRedirect != NULL){    
                inputRedirect(pCmdLine);
            }
            if (pCmdLine->outputRedirect != NULL){
                outputRedirect(pCmdLine);
            }

            stat = execvp(pCmdLine->arguments[0],pCmdLine->arguments);
            perror("execv failed!");
            freeCmdLines(pCmdLine); //not supposed to get here if execute succeeded?
            exit(1);
        }
        else{
            //in the parent process
            addProcess(process_list, pCmdLine, pid);
            if(pCmdLine->blocking == 1){
                waitpid(pid,&stat,0); 
            }
            if (debug == 1)
                fprintf(stderr, "PID: %d, executing: %s\n",pid, pCmdLine->arguments[0]);
        }
    }
    if (succ == -1)
        perror("operation fail!" );
}


int main(int argc, char **argv){
    //check dibug flag
    for (int i = 1; i < argc; i++){ 
        if(strcmp(argv[i],"-d") == 0){
            debug =1; 
            break;
        }
    }

    process **process_list = (process **)malloc(sizeof(process **));
    *process_list = NULL;

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
            // if (strcmp(userInput,"!!") != 0 && !(userInput[0] == '!' && atoi(userInput[1]) >= 0 && atoi(userInput[1]) <= HISTLEN)) {
            //     addHistory(userInput);
            // }
            if (strcmp(userInput, "!!\n") != 0 && !(userInput[0] == '!' && isdigit(userInput[1]) != 0 && atoi(&userInput[1]) >= 0 && atoi(&userInput[1]) <= HISTLEN)) {
                addHistory(userInput);
            }
            // if (userInput[0] != '!') {
            //     addHistory(userInput);
            // }
            //parse the input
            inputAfterParse = parseCmdLines(userInput);  
            if(inputAfterParse != NULL){
                if(strcmp(inputAfterParse->arguments[0],"quit") == 0){
                    freeCmdLines(inputAfterParse);
                    freeProcessList(*process_list);
                    free(process_list);
                    freeHistory();
                    exit(0);
                }
                execute(inputAfterParse, process_list);
            }
        }
    }
    return 0; 
}