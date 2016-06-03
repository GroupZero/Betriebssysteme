/*
 Abgabe von:
 Max Mayerhofer (550278)
 Phillipp Geimer (563681)
 Feliks Scholzef (555155)
 Sonay Senguen (550534)
 Piotr Kakeh (550621)
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_CHARACTER_INPUT 256
#define MAX_FOLDER 10
#define MAX_FOLDER_NAME 15
#define MAX_BUFFER 180
#define CHILD_RET 17
#define MAX_BACKGROUND_PROCESSES 10

//cd zeug
unsigned int startPos = 0;

pid_t waitID_activ = NULL;
int pipeOn = 0;
char *arg1;
char *arg2;
pid_t *arrayWithBackgroundProcesses;


char* getPath() {
    // get pwd
    char *pwd = malloc(sizeof(char)*MAX_CHARACTER_INPUT);
    getcwd(pwd,MAX_BUFFER);
    if (strlen(pwd) > startPos) {
        //printf("all: %s\nlast[%d]: %s\n",pwd,startPos,pwd+startPos);
        return pwd+startPos;
    }
    free(pwd);
    
    return "";
}

void addBackgroundProcess(pid_t proc_id) {
    arrayWithBackgroundProcesses[backgroundPorcesses] = proc_id;
    backgroundPorcesses++;
}

void deleteBackgroundProcess(pid_t proc_id) {
    int i=0;
    while (i<backgroundPorcesses) {
        if (arrayWithBackgroundProcesses[i] == proc_id) {
            // override it with the last array entry
            if (i == backgroundPorcesses-1) {
                arrayWithBackgroundProcesses[i] = 0;
            } else {
                arrayWithBackgroundProcesses[i] = arrayWithBackgroundProcesses[backgroundPorcesses-1];
            }
            i=backgroundPorcesses;
        }
        i++;
    }
    backgroundPorcesses--;
}

void waitForOnePID(pid_t waitID) {
        int stat;
        waitID_activ = waitID
        printf("Waiting for Process with ID:%d ...\n",waitID);
        waitpid(waitID,&stat,0);
       if(waitID_activ!=NULL){
        printf("ProcessWithID:%d\nWIFEXITED:%d\nWIFISIGNALED:%d\nWTERMSIG:%d\nWEXITSTATUS:%d\n\n",waitID,WIFEXITED(stat),WIFSIGNALED(stat),WTERMSIG(stat),WEXITSTATUS(stat));
       }else{
        printf("Wait was cancled.\n");
       }
}

void waitCommand(const char* inputArgs) {
    // String mit den zu wartenden PIDs mit Leerzeichen getrennt 
    char *words = strtok((char*)inputArgs, " ");
    while (words != NULL) {
        //die nächste PidID
        pid_t child_id = (pid_t)atoi(words);
        //wartet auf den PID
        waitForOnePID(child_id);
        // words wird zur nächsten PID ID 
        words = strtok(NULL," ");
    }
}

void setStartPos() {
    // get pwd
    char *pwd = malloc(sizeof(char)*MAX_CHARACTER_INPUT);
    getcwd(pwd,MAX_BUFFER);
    // if (strlen(pwd) < startPos)
    
    // get length of pwd
    startPos = (int)strlen(pwd);
    free(pwd);
}

void cd(const char* command) {
    // maybe we want to jump in a folder over our home directory
    // test it on the first char, that should be /
    if (strncmp("/", command+3, 1) == 0) {
        // first char is /, now we have to search for the folder
        // compare with pwd
        char *pwd = malloc(sizeof(char)*MAX_CHARACTER_INPUT);
        getcwd(pwd,MAX_BUFFER);
        char* path = strstr(pwd,command+3);
        if (path) {
            char *newDirectory = malloc(sizeof(char)*startPos);
            
            int pos = (int)(path - pwd);
            int path_length = (int)(pos+strlen(command+3));
            
            
            memcpy(newDirectory, pwd, path_length);
            
            
            //newDirectory[path_length] = '\0';
            if (chdir(newDirectory) == 0) {
                setStartPos();
            } else {
                printf("No Folder with the Name: %s\n",command+3);
            }
            
            free(newDirectory);
        } else {
            printf("No Folder with the Name: %s\n",command+3);
            return;
        }
        free(pwd);
    } else {
        if (chdir(command+3)) {
            printf("No Folder with the Name: %s\n",command+3);
        }
        
        return;
    }
}

char** trimCommand(const char* input) {
    char **args = NULL;
    int n=0;
    char *words = strtok((char*)input, " ");
    
    while (words != NULL) {
        if (n) { // Array args not empty
            args = realloc(args,(n+1)*sizeof(char**));
        } else { // first initialisation
            args = malloc((n+1)*sizeof(char*));
        }
        
        // check initialisation
        if (!args) {
            perror("Error: Initialisation");
            free(words);
            exit(-1);
        }
        
        args[n] = malloc(strlen(words)+1);
        // check initialisation
        if (!args[n]) {
            perror("Error: Initialisation");
            free(words);
            exit(-1);
        }
        strcpy(args[n] , words);
        n++;
        words = strtok(NULL," ");
    }
    free(words);
    
    
    
    // add NULL
    if (n) { // Array args not empty
        args = realloc(args,(n+1)*sizeof(char**));
    } else { // first initialisation
        args = malloc((n+1)*sizeof(char*));
    }
    args[n] = NULL;
    
    return args;
}

int runProgram(const char* _input) {
    char *input = malloc(sizeof(char)*strlen(_input));
    int background = 0;
    if (strcmp(_input+strlen(_input)-2," &") == 0) {
        background = 1;
        memcpy(input, _input, strlen(_input)-2);
        input[strlen(_input)-2] = '\0';
    } else memcpy(input, _input, strlen(_input));
    
    
    char **args = NULL;
    int n=0;
    
    pid_t proc_id;
    int status = 0;
    
    
    int pdes[2];
    pipe(pdes);
    
    proc_id = fork();
    if (proc_id < 0) {
        fprintf(stderr, "fork error\n");
        fflush(stderr);
        return EXIT_FAILURE;
    }
    if (proc_id == 0) { /* child process */
        
        if (pipeOn == 0) {
            args = trimCommand((char*)input);
        } else {
            args = trimCommand(arg1);
            /* child process */
            close(pdes[0]);
            close(1);       /* close stdout */
            dup(pdes[1]);
        }
        // execute
        if (background) {
            fclose(stdin); // close stdin for this child
            fopen("dev/null", "r"); // open new stdin, that is always empty
        }
        execvp(args[0], args);
        exit(1);
        
    } else { /* parent */
        
        if (pipeOn == 1) {
            args = trimCommand(arg2);
            close(0);       /* close stdin */
            close(pdes[1]);
            
            dup(pdes[0]);
            execvp(args[0], args);
        }
        // free array
        /* int i=0;
         while (i<n) {
         //printf("n=%d, i=%d\n",n,i);
         free(args[i]);
         i++;
         }*/
        free(args);
        
        if (background) {
            addBackgroundProcess(proc_id);
            printf("[%d]\n",proc_id);
        } else {
            waitpid(proc_id, &status, 0);
        }
    }
    return EXIT_SUCCESS;
}

void doWork(const char* command) {
    pipeOn = 0;
    // check whether is | in command
    if (strstr(command, "|") == 0) { // no pipe
        if (strncmp("exit",command,4) == 0) {
            exit(0);
            return;
        }
        if (strncmp("wait",command,3) == 0) {
           //führt wait aus 
            waitCommand(command+4);
            return;
        }
        if (strncmp("cd",command,2) == 0) {
            cd(command);
            return;
        }
        
        //if (!runProgram(command)) printf("Error\n");
        runProgram(command);
    } else { // pipe
        pipeOn = 1;
        char *words = strtok((char*)command, "|");
        arg1 = words;
        words = strtok(NULL,"|");
        arg2 = words;
        
        // build pipe
        runProgram(command);
    }
    
    
}


void event_strg_c(int num){
    if (waitID_activ != NULL) {
        kill(waitID_activ, 16);
        waitID_activ = NULL;
    }
}

int main(void)
{
    arrayWithBackgroundProcesses = malloc(sizeof(pid_t)*MAX_BACKGROUND_PROCESSES); // set max processes
    arrayWithWaitProcesses = malloc(sizeof(pid_t)*MAX_BACKGROUND_PROCESSES); // set max wait processes
    setStartPos();
    
    //chat Str+C
    signal(SIGINT,event_strg_c);
    
    printf(".>");
    
    
    
    // read Input
    while (1) {
        char *command = malloc(sizeof(char)*MAX_CHARACTER_INPUT);
        if (fgets(command, MAX_BUFFER, stdin)) {
            if (strlen(command) == 0) { // Error: no input
                printf("Error: no input.\n");
            } else {
                strtok(command,"\n");
                doWork(command);
            }
            printf(".%s>",getPath());
        }
        free(command);
    }
    
    return 0;
}


