#ifndef LAB4COMMON
#define LAB4COMMON

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 7788
#define EPOLL_SIZE 5000
#define BUF_SIZE 0xFFFF
#define SERVER_WELCOME "Welcome you join to the game! Your ID is: %s\n"
#define SERVER_MULTI_ID "This ID has been used by another user.\n"
#define SERVER_LEAVE "User %s has left the game.\n"
#define CLIENT_HP 100
#define CLIENT_DMG 40

struct Msg
{
    int type;
    char fromID[10];
    char toID[10];
    char content[BUF_SIZE];
};
static void addfd( int epollfd, int fd);

#endif