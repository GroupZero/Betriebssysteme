#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 9000
#define BUF_SIZE 128
#define BUF_SIZE_OUTPUT 99999


#include <sys/types.h>
#include <sys/wait.h>

#define MAX_CHARACTER_INPUT 256
#define MAX_FOLDER 10
#define MAX_FOLDER_NAME 15
#define CHILD_RET 17
#define MAX_BACKGROUND_PROCESSES 10

int srv_sock;
int cli_sock;

// +++++++++++++++ SHELL STUFF STARTS +++++++++++++++++++++
//cd zeug
unsigned int startPos = 0;

pid_t waitID_activ = 0;
int pipeOn = 0;
char *arg1;
char *arg2;
pid_t *arrayWithBackgroundProcesses;
int backgroundPorcesses = 0;

static void term(const char *msg)
{
    if (msg) {
        fprintf(stderr, "Error: %s\n", msg);
        fflush(stderr);
    }
    
    int res;
    
    res = close(srv_sock);
    if (res < 0) {
        fprintf(stderr, "Error: couldn't close server socket\n");
        fflush(stderr);
    }
    
    res = close(cli_sock);
    if (res < 0) {
        fprintf(stderr, "Error: couldn't close server socket\n");
        fflush(stderr);
    }
    
    if (msg)
        exit(EXIT_FAILURE);
    else
        exit(EXIT_SUCCESS);
}

void sendToClient(char* input) {
    int ret = write(cli_sock, input, ret);
    if (ret < 0)
        term("Couldn't write to socket");
}

char* getPath() {
    // get pwd
    char *pwd = malloc(sizeof(char)*MAX_CHARACTER_INPUT);
    getcwd(pwd,BUF_SIZE);
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
    
    char *temp = malloc(sizeof(char)*MAX_CHARACTER_INPUT);
    sprintf(temp,"Waiting for Process with ID:%d ...\n",waitID);
    sendToClient(temp);
    free(temp);
    
    waitID_activ = waitID;
    waitpid(waitID,&stat,0);
    if(waitID_activ==0){
        sprintf(temp,"you pressed ctrl + c, thats why we kill the process and stops your waiting. Now here are the results: \n");
        sendToClient(temp);
    }else{
        sprintf(temp,"your waiting was worth it. Here are the results of my study: \n");
        sendToClient(temp);
    }
    sprintf(temp,"ProcessWithID:%d\n",waitID);
    sendToClient(temp);
    
    sprintf(temp,"\tends normaly:%d\n",WIFEXITED(stat));
    sendToClient(temp);
    
    //Durch signal beendet
    if(WIFSIGNALED(stat)){
        sprintf(temp,"\tEnd by the signal: %d\n",WTERMSIG(stat));
        sendToClient(temp);
    }else{
        sendToClient("\tWas not killed by a signal. \n");
    }
    
    sprintf(temp,"\treturn of process: %d\n\n",WEXITSTATUS(stat));
    sendToClient(temp);
    free(temp);
    
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
    getcwd(pwd,BUF_SIZE);
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
        getcwd(pwd,BUF_SIZE);
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
                char *temp = malloc(sizeof(char)*BUF_SIZE_OUTPUT);
                sprintf(temp,"No Folder with the Name: %s\n",command+3);
                sendToClient(temp);
                free(temp);
            }
            
            free(newDirectory);
        } else {
            char *temp = malloc(sizeof(char)*BUF_SIZE_OUTPUT);
            sprintf(temp,"No Folder with the Name: %s\n",command+3);
            sendToClient(temp);
            free(temp);
            return;
        }
        free(pwd);
    } else {
        if (chdir(command+3)) {
            char *temp = malloc(sizeof(char)*BUF_SIZE_OUTPUT);
            sprintf(temp,"No Folder with the Name: %s\n",command+3);
            sendToClient(temp);
            free(temp);
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
    
    
    // Redirect IO
    dup2(cli_sock, 0);
    dup2(cli_sock, 1);
    dup2(cli_sock, 2);
    
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
        signal(2, SIG_IGN);
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
            
            // Redirect IO
            dup2(cli_sock, 0);
            dup2(cli_sock, 1);
            dup2(cli_sock, 2);
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
            //addBackgroundProcess(proc_id);
            
            char *temp = malloc(sizeof(char)*BUF_SIZE_OUTPUT);
            sprintf(temp,"[%d]\n",proc_id);
            sendToClient(temp);
            free(temp);
        } else {
            waitID_activ= proc_id;
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
    if (waitID_activ != 0) {
        kill(waitID_activ, 16);
        
        char *temp = malloc(sizeof(char)*BUF_SIZE_OUTPUT);
        sprintf(temp,"\nprocess with ID:%d was killed\n", waitID_activ);
        sendToClient(temp);
        free(temp);
        
        waitID_activ = 0;
    }
}
// +++++++++++++++ SHELL STUFF ENDS +++++++++++++++++++++



static void handle_signal(int signo)
{
    term(NULL);
}

static void handle_req(const char *cmd)
{
    fprintf(stdout, "received: %s\n", cmd);
    fflush(stdout);
}

static void sendData() {
    char buf[BUF_SIZE];
    int ret;
    while (1) {
        /* prepare buffer */
        memset(buf, 0, BUF_SIZE);
        
        /* get request */
        ret = read(cli_sock, buf, BUF_SIZE - 1); /* -1: '\0' ending */
        
        if (ret < 0)
            term("Couldn't read from socket");
        
        /* handle no input, e.g. client exit during open connection */
        if (ret == 0)
            break;
        
        /* handle request */
        //handle_req(buf);
        
        /* ++++++++++++ SHELL STARTS +++++++++++++++ */
        setStartPos();
        
        //chat Str+C
        signal(SIGINT,event_strg_c);
        
        
        
        // read Input
        
        char *command = malloc(sizeof(char)*BUF_SIZE_OUTPUT);
        strcpy(command, buf);
        if (strlen(command) == 0) { // Error: no input
            printf("Error: no input.\n");
        } else {
            strtok(command,"\n");
            doWork(command);
        }
        
        char *path_temp = malloc(sizeof(char)*BUF_SIZE_OUTPUT);
        sprintf(path_temp,".%s>",getPath());
        ret = write(cli_sock, path_temp, ret);
        if (ret < 0)
            term("Couldn't write to socket");
        free(path_temp);
        free(command);
        
        
        /* +++++++++++++ SHELL ENDS ++++++++++++++++++ */
        
        
        
        /* send response */
        
        //sprintf(buf,"%d",getpid()); // send the process id back to client
        //  ret = write(cli_sock, buf, ret);
        
        
    }
}


int main(void)
{
    int ret;
    
    struct sockaddr_in srv_addr;
    struct sockaddr_in cli_addr;
    
    size_t sockaddr_sz = sizeof(struct sockaddr_in);
    
    
    /* register custom signal handler */
    if (signal(SIGINT, handle_signal) == SIG_ERR)
        term("Couldn't register custom signal handler");
    
    
    /* prepare server address */
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port   = htons(PORT);
    srv_addr.sin_addr.s_addr = INADDR_ANY;
    
    
    /* open socket */
    srv_sock = socket(AF_INET, SOCK_STREAM, 0);
    
    if (srv_sock < 0)
        term("Couldn't open socket");
    
    
    
    /* set socket options for address reuse */
    int n = 1;
    ret = setsockopt(srv_sock, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(int));
    
    if (ret < 0)
        term("Couldn't set socket options");
    
    
    /* bind socket to server address */
    ret = bind(srv_sock, (struct sockaddr*) &srv_addr, sockaddr_sz);
    
    if (ret < 0)
        term("Couldn't bind socket to server address");
    
    
    /* start listening for incoming connections */
    ret = listen(srv_sock, 1);
    
    if (ret < 0)
        term("Couldn't listen using the socket");
    
    while (1) {
        /* open client socket for incoming connection */
        cli_sock = accept(srv_sock,
                          (struct sockaddr*) &cli_addr,
                          (socklen_t *) &sockaddr_sz);
        
        if (cli_sock < 0)
            term("Couldn't accept incoming connection");
        
        int proc_id = fork();
        
        if (proc_id < 0) {
            perror("fork error\n");
        }
        if (proc_id == 0) {
            sendData();
        }
        
    }
    
    /* clean up */
    term(NULL);
    
    return EXIT_SUCCESS;
}
