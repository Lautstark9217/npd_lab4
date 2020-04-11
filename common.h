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

#define STAT_OFF 0
#define STAT_WAITID 1
#define STAT_FREE 2
#define STAT_DUEL 3

#define MSG_IDQUERY 0
#define MSG_IDOK 1
#define MSG_DUPID 2
#define MSG_CHALQUERY 3
#define MSG_CHAL_REP_INCHAL 4
#define MSG_CHAL_REP_OK 5
#define MSG_CHAL_NODMG 6
#define MSG_CHAL_DMG 7
#define MSG_CHAL_PUT 12
#define MSG_LOSE 8
#define MSG_WIN 9
#define MSG_LIST 10
#define MSG_CHALREQ_DECL 11


struct _Msg
{
    int type;
    char fromID[10];
    char toID[10];
    char content[BUF_SIZE];
};
typedef struct _Msg Msg;

static void addfd( int epollfd, int fd)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0)| O_NONBLOCK);
    //printf("fd added to epoll\n\n");
}
#endif