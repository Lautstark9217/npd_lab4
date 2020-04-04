#ifndef LAB4CLIENT
#define LAB4CLIENT
#include "common.h"
struct Client {
    // connect to a server
    void clientConnect();
    // close connection
    void clientClose();
    // start a client program
    void clientStart();
    // current sock fd
    int sockfd;
    // current thread id
    int pid;
    // epoll_create return
    int epfd;
    // pipe connect
    int pipe_fd[2];
    //whether online
    bool isClientwork;
    //whether in duel
    bool isClientinDuel;
    Msg msg;
    char send_buf[BUF_SIZE];
    char recv_buf[BUF_SIZE];
    struct sockaddr_in serverAddr;
};

#endif