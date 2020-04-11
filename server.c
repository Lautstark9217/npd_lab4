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
    int on=1;  
    if((setsockopt(server->listener,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)  
    {  
        perror("setsockopt failed");  
        exit(EXIT_FAILURE);  
    }  
    if(bind(server->listener, (struct sockaddr *)&(server->serverAddr), sizeof(server->serverAddr)) < 0) {
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
int serverSendMsg(Server* server,int clientfd)
{
    // buf[BUF_SIZE] 接收新消息
    // message[BUF_SIZE] 保存格式化的消息
    char recv_buf[BUF_SIZE];
    char send_buf[BUF_SIZE];
    Msg msg;
    bzero(recv_buf, BUF_SIZE);
    // 接收新消息
    printf("read from client(clientID = %d )\n",clientfd);
    int len = recv(clientfd, recv_buf, BUF_SIZE, 0);
    //清空结构体，把接受到的字符串转换为结构体
    memset(&msg,0,sizeof(msg));
    memcpy(&msg,recv_buf,sizeof(msg));
    msg.fromID=clientfd;
    if(len == 0) 
    {
        close(clientfd);
        // 在客户端列表中删除该客户端
        for(int i=0;i<server->pList;i++)
        {
            if(server->List[i].sockfd==clientfd)
            {
                strcpy(server->List[i].userID,"");
                server->List[i].stat=STAT_OFF;
                return len;
            }
        }
    }
    int ret_msgtype=-1;
    switch (msg.type)
    {
    case MSG_IDQUERY:
        msg.toID=clientfd;
        for(int i=0;i<server->pList;++i)
        {
            if(strcmp(msg.content,server->List[i].userID)==0)
                ret_msgtype=MSG_DUPID;
        }
        if(ret_msgtype!=MSG_DUPID)
        {
            ret_msgtype=MSG_IDOK;
            for(int i=0;i<server->pList;++i)
            {
                if(server->List[i].sockfd==clientfd)
                {
                    strcpy(server->List[i].userID,msg.content);
                    break;
                }
            }
        }
        msg.type=ret_msgtype;
        bzero(send_buf, BUF_SIZE);
        memcpy(send_buf,&msg,sizeof(msg));
        if( send(clientfd,send_buf, sizeof(send_buf), 0) < 0 ) {
            return -1;
        }
        break;
    
    default:
        break;
    }
    //判断接受到的信息是私聊还是群聊
    // msg.fromID=clientfd;
    // if(msg.content[0]=='\\'&&isdigit(msg.content[1])){
    //     msg.type=1;
    //     msg.toID=msg.content[1]-'0';
    //     memcpy(msg.content,msg.content+2,sizeof(msg.content));
    // }
    // else
    //     msg.type=0;
    // // 发送广播消息给所有客户端
    // else 
    // {
    //     //存放格式化后的信息
    //     char format_message[BUF_SIZE];
    //     //群聊
    //     if(msg.type==0){
    //         // 格式化发送的消息内容 #define SERVER_MESSAGE "ClientID %d say >> %s"
    //         sprintf(format_message, SERVER_MESSAGE, clientfd, msg.content);
    //         memcpy(msg.content,format_message,BUF_SIZE);
    //         // 遍历客户端列表依次发送消息，需要判断不要给来源客户端发
    //         list<int>::iterator it;
    //         for(it = clients_list.begin(); it != clients_list.end(); ++it) {
    //            if(*it != clientfd){
    //                 //把发送的结构体转换为字符串
    //                 bzero(send_buf, BUF_SIZE);
    //                 memcpy(send_buf,&msg,sizeof(msg));
    //                 if( send(*it,send_buf, sizeof(send_buf), 0) < 0 ) {
    //                     return -1;
    //                 }
    //            }
    //         }
    //     }
    //     //私聊
    //     if(msg.type==1){
    //         bool private_offline=true;
    //         sprintf(format_message, SERVER_PRIVATE_MESSAGE, clientfd, msg.content);
    //         memcpy(msg.content,format_message,BUF_SIZE);
    //         // 遍历客户端列表依次发送消息，需要判断不要给来源客户端发
    //         list<int>::iterator it;
    //         for(it = clients_list.begin(); it != clients_list.end(); ++it) {
    //            if(*it == msg.toID){
    //                 private_offline=false;
    //                 //把发送的结构体转换为字符串
    //                 bzero(send_buf, BUF_SIZE);
    //                 memcpy(send_buf,&msg,sizeof(msg));
    //                 if( send(*it,send_buf, sizeof(send_buf), 0) < 0 ) {
    //                     return -1;
    //                 }
    //            }
    //         }
    //         //如果私聊对象不在线
    //         if(private_offline){
    //             sprintf(format_message,SERVER_PRIVATE_ERROR_MESSAGE,msg.toID);
    //             memcpy(msg.content,format_message,BUF_SIZE);
    //             bzero(send_buf,BUF_SIZE);
    //             memcpy(send_buf,&msg,sizeof(msg));
    //             if(send(msg.fromID,send_buf,sizeof(send_buf),0)<0)
    //                 return -1;
    //         }
    //     }
    // }
    return len;
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
                strcpy(server->List[server->pList].userID,"");
                server->pList++;
                printf("Add new clientfd = %d to epoll\n",clientfd); 
                char message[BUF_SIZE];
                bzero(message, BUF_SIZE);
                sprintf(message, SERVER_WELCOME, "test");
                int ret = send(clientfd, message, BUF_SIZE, 0);
                if(ret < 0) {
                    perror("send error");
                    serverClose(server);
                    exit(-1);
                }
            }
            //处理用户发来的消息，并广播，使其他用户收到信息
            else {   
                int ret = serverSendMsg(server,sockfd);
                if(ret < 0) {
                    perror("error");
                    serverClose(server);
                    exit(-1);
                }
            }
        }
    }
    // 关闭服务
    serverClose(server);
}