#ifndef LAB4SERVER
#define LAB4SERVER
#include "common.h"
struct _clientlist
{
    char userID[10];
    int sockfd;
    int stat;
};
typedef struct _clientlist clientList;
struct _Server
{
    struct sockaddr_in serverAddr;
    int listener;
    int epfd;
    clientList List[20];
    int pList;
};
typedef struct _Server Server;

void serverInit(Server* server);
void serverClose(Server* server);
void serverStart(Server* server);
void serverSendMsg(Server* server);

#endif