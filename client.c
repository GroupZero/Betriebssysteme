#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define HOST "127.0.0.1"
#define PORT 9000

#define BUF_SIZE 128

int sock;

static void term(const char *msg)
{
    if (msg) {
        fprintf(stderr, "Error: %s\n", msg);
        fflush(stderr);
    }
    
    int res = close(sock);
    
    if (res < 0) {
        fprintf(stderr, "Error: couldn't close socket\n");
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

int main(void)
{
    int ret;
    char buf[BUF_SIZE];
    
    struct sockaddr_in srv_addr;
    size_t sockaddrin_sz = sizeof(struct sockaddr_in);
    
    
    /* register custom signal handler */
    if (signal(SIGINT, handle_signal) == SIG_ERR)
        term("Couldn't register custom signal handler");
    
    
    /* prepare server address */
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(PORT);
    srv_addr.sin_addr.s_addr = inet_addr(HOST);
    
    
    /* open socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sock < 0)
        term("Couldn't open socket");
    
    
    /* connect to socket */
    ret = connect(sock, (struct sockaddr*) &srv_addr, sockaddrin_sz);
    
    if (ret < 0)
        term("Couldn't connect to socket");
    
    
    size_t cmd_sz = BUF_SIZE - 1;
    char *cmd = buf;
    printf(".>");
    while (1) {
        /* get user command */
        ret = getline(&cmd, &cmd_sz, stdin);
        
        if (ret < 0)
            term("Couldn't get command from user");
        
        buf[ret] = '\0';
        
        /* send command request to server */
        ret = write(sock, buf, ret);
        
        if (ret < 0)
            term("Couldn't write to socket");
        
        /* exit if required */
        if (!strcmp("exit\n", buf))
            break;
        
        /* get server response */
        memset(buf, 0, BUF_SIZE);
        
        ret = read(sock, buf, BUF_SIZE - 1); /* -1 for '\0' ending */
        
        if (ret < 0)
            term("Couldn't read from socket");
        
        /* output response */
        fprintf(stdout,"%s", buf);
        fflush(stdout);
    }
    
    term(NULL);
    
    return EXIT_SUCCESS;
}
