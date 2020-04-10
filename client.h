#ifndef LAB4CLIENT
#define LAB4CLIENT
#include "common.h"
struct _Client {
    // current sock fd
    int sockfd;
    // current thread id
    int pid;
    // epoll_create return
    int epfd;
    // pipe connect
    int pipe_fd[2];
    //s
    int stat;

    Msg msg;
    char send_buf[BUF_SIZE];
    char recv_buf[BUF_SIZE];
    char myid[10];
    struct sockaddr_in serverAddr;
};
typedef struct _Client Client;

// init a client
void clientInit(Client*);
// connect to a server
void clientConnect(Client*);
// close connection
void clientClose(Client*);
// start a client program
void clientStart(Client*);


#endif