#include "server.h"

void serverInit(Server* server)
{
    server->serverAddr.sin_family = PF_INET;
    server->serverAddr.sin_port = htons(SERVER_PORT);
    server->serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server->listener = 0;
    server->epfd = 0;
    server->pList=0;
    server->listener = socket(PF_INET, SOCK_STREAM, 0);
    if(server->listener < 0) { perror("listener"); exit(-1);}
    if( bind(server->listener, (struct sockaddr *)&(server->serverAddr), sizeof(server->serverAddr)) < 0) {
        perror("bind error");
        exit(-1);
    }
    //监听
    int ret = listen(server->listener, 5);
    if(ret < 0) {
        perror("listen error"); 
        exit(-1);
    }
    //在内核中创建事件表 epfd是一个句柄 
    server->epfd = epoll_create (EPOLL_SIZE);
    if(server->epfd < 0) {
        perror("epfd error");
        exit(-1);
    }
    addfd(server->epfd, server->listener);
}
void serverClose(Server* server)
{
    close(server->listener);
    close(server->epfd);
}
void serverStart(Server* server)
{
    static struct epoll_event events[EPOLL_SIZE]; 
    serverInit(server);
    while(1)
    {
        //epoll_events_count表示就绪事件的数目
        int epoll_events_count = epoll_wait(server->epfd, events, EPOLL_SIZE, -1);

        if(epoll_events_count < 0) {
            perror("epoll failure");
            break;
        }
        printf("epoll_events_count = %d\n",epoll_events_count);

        //处理这epoll_events_count个就绪事件
        for(int i = 0; i < epoll_events_count; ++i)
        {
            int sockfd = events[i].data.fd;
            //新用户连接
            if(sockfd == server->listener)
            {
                struct sockaddr_in client_address;
                socklen_t client_addrLength = sizeof(struct sockaddr_in);
                int clientfd = accept(server->listener, ( struct sockaddr* )&client_address, &client_addrLength );
                printf("client connection from: \n");
                addfd(server->epfd, clientfd);
                server->List[server->pList].sockfd=clientfd;
                server->List[server->pList].stat=STAT_FREE;
                printf("Add new clientfd = %d to epoll\n",clientfd); 
                char message[BUF_SIZE];
                bzero(message, BUF_SIZE);
                sprintf(message, SERVER_WELCOME, clientfd);
                int ret = send(clientfd, message, BUF_SIZE, 0);
                if(ret < 0) {
                    perror("send error");
                    Close();
                    exit(-1);
                }
            }
            //处理用户发来的消息，并广播，使其他用户收到信息
            else {   
                int ret = SendBroadcastMessage(sockfd);
                if(ret < 0) {
                    perror("error");
                    Close();
                    exit(-1);
                }
            }
        }
    }

    // 关闭服务
    serverClose(server);
}