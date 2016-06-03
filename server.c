#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 9000
#define BUF_SIZE 128

int srv_sock;
int cli_sock;


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
        handle_req(buf);
        
        if (!strcmp("exit\n", buf))
            break;
        
        /* send response */
        
        sprintf(buf,"%d",getpid()); // send the process id back to client
        ret = write(cli_sock, buf, ret);
        
        if (ret < 0)
            term("Couldn't write to socket");
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
