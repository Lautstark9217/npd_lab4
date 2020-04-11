#include "client.h"

void clientInit(Client* client)
{
    client->serverAddr.sin_family = PF_INET;
    client->serverAddr.sin_port = htons(SERVER_PORT);
    client->serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    client->sockfd = 0;
    client->pid = 0;
    client->stat=STAT_WAITID;
    client->epfd = 0;
    client->receive_decide=0;
}
void clientConnect(Client* client)
{
    printf("Connecting to server: %s : %d\n",SERVER_IP,SERVER_PORT);
    client->sockfd=socket(PF_INET,SOCK_STREAM,0);
    if(client->sockfd < 0) {
        perror("sock error");
        exit(-1); 
    }
    // 连接服务端
    if(connect(client->sockfd, (struct sockaddr *)&(client->serverAddr), sizeof(client->serverAddr)) < 0) {
        perror("connect error");
        exit(-1);
    }
    // 创建管道，其中fd[0]用于父进程读，fd[1]用于子进程写
    if(pipe(client->pipe_fd) < 0) {
        perror("pipe error");
        exit(-1);
    }
    // 创建epoll
    client->epfd = epoll_create(EPOLL_SIZE);
    if(client->epfd < 0) {
        perror("epfd error");
        exit(-1); 
    }
    //将sock和管道读端描述符都添加到内核事件表中
    addfd(client->epfd, client->sockfd);
    addfd(client->epfd, client->pipe_fd[0]);
}
void clientClose(Client* client)
{
    if(client->pid)
    {
        close(client->pipe_fd[0]);
        close(client->sockfd);
    }
    else close(client->pipe_fd[1]);
}
void _clientStart(Client* client)
{
    static struct epoll_event ev[2];
    clientConnect(client);
    client->pid=fork();
    if(client->pid < 0) {
        perror("fork error");
        close(client->sockfd);
        exit(-1);
    } else if(client->pid == 0) {
        // 进入子进程执行流程
        //子进程负责写入管道，因此先关闭读端
        close(client->pipe_fd[0]); 
        printf("Please type 'exit' to exit the arena.\nPlease type the ID of who you want to challenge to start a game.");
        printf("Please type 'rank' to get the current ranking\n");
        printf("When you are in game,type numbers:\n");
        printf("1-rock\n");
        printf("2-paper\n");
        printf("3-scissors\n");
        printf("Have fun!\n");
        // 如果客户端运行正常则不断读取输入发送给服务端
        while(client->stat){
            //清空结构体
            memset(client->msg.content,0,sizeof(client->msg.content));
            fgets(client->msg.content, BUF_SIZE, stdin);
            // 客户输出exit,退出
            if(strncasecmp(client->msg.content, "exit", strlen("exit")) == 0 && client->stat!=STAT_DUEL){
                client->stat = STAT_OFF;
            }
            // 子进程将信息写入管道
            else {
                switch (client->stat)
                {
                    case STAT_WAITID:
                        printf("Please type your ID:\n");
                        scanf("%s",client->msg.content);
                        client->msg.type=MSG_IDQUERY;
                        break;
                    case STAT_FREE:
                        printf("Please type the ID you want to challenge:\n");
                        scanf("%s",client->msg.content);
                        client->msg.type=MSG_CHALQUERY;
                        break;
                    case STAT_DUEL:
                        do
                        {
                           scanf("%s",client->msg.content);
                           if(strlen(client->msg.content)==1 && client->msg.content[0]>'0' && client->msg.content[0]<'4') break;
                           else printf("Illegal put!\n");
                        } while (1);
                        client->msg.type=MSG_CHAL_PUT;
                        break;
                    default:
                    break;
                }
                //清空发送缓存
                memset(client->send_buf,0,BUF_SIZE);
                //结构体转换为字符串
                memcpy(client->send_buf,&(client->msg),sizeof(client->msg));
                if( write(client->pipe_fd[1], client->send_buf, sizeof(client->send_buf)) < 0 ) { 
                    perror("fork error");
                    exit(-1);
                }
            }
        }
    } else { 
        //pid > 0 父进程
        //父进程负责读管道数据，因此先关闭写端
        close(client->pipe_fd[1]); 
        // 主循环(epoll_wait)
        while(client->stat) {
            int epoll_events_count = epoll_wait(client->epfd, ev, 2, -1 );
            //处理就绪事件
            for(int i = 0; i < epoll_events_count ; ++i)
            {
                memset(client->recv_buf,0,sizeof(client->recv_buf));
                //服务端发来消息
                if(ev[i].data.fd == client->sockfd)
                {
                    //接受服务端广播消息
                    int ret = recv(client->sockfd, client->recv_buf, BUF_SIZE, 0);
                    //清空结构体
                    memset(&(client->msg),0,sizeof(client->msg));
                    //将发来的消息转换为结构体
                    memcpy(&(client->msg),client->recv_buf,sizeof(client->msg));
                    // ret= 0 服务端关闭
                    if(ret == 0) {
                        printf("Server closed connection: %d\n",client->sockfd);
                        close(client->sockfd);
                        client->stat = 0;
                    }
                    else 
                    {
                        switch (client->stat)
                        {
                            case STAT_WAITID:
                                if(client->msg.type==MSG_IDOK)
                                {
                                    printf("You have entered the arena.\n");
                                    client->stat=STAT_FREE;
                                }
                                else if(client->msg.type==MSG_DUPID)
                                    printf("This ID has been taken by other user.\nPlease choose another one:\n");
                                break;
                            case STAT_FREE:
                                switch (client->msg.type)
                                {
                                case MSG_CHAL_REP_INCHAL:
                                    printf("This person is currently in another game.\n");
                                    printf("Please choose another person.\n");
                                    break;
                                case MSG_CHAL_REP_OK:
                                    printf("You are about to challenge user ID: %s!\n",client->msg.content);
                                    client->stat=STAT_DUEL;
                                    break;
                                case MSG_CHALREQ_DECL:
                                    printf("This user has declined your invite\n");
                                case MSG_CHALQUERY:
                                    printf("You are challenged by user ID: %s.\n",client->msg.content);
                                    printf("Please type Y to accept, or N to decline.\n");
                                    do
                                    {
                                      char p;
                                      scanf("%c",&p);
                                      if(p=='Y')
                                      {
                                          client->receive_decide=1;
                                          break;
                                      }
                                      else if(p=='N')
                                      {
                                          client->receive_decide=2;
                                          break;
                                      }
                                      else printf("Please type Y/N!\n");
                                    } while (1);
                                    
                                default:
                                    break;
                                }
                            default:
                                break;
                        }
                    }
                }
                //子进程写入事件发生，父进程处理并发送服务端
                else { 
                    //父进程从管道中读取数据
                    int ret = read(ev[i].data.fd, client->recv_buf, BUF_SIZE);
                    // ret = 0
                    if(ret == 0)
                        client->stat = STAT_OFF;
                    else {
                        // 将从管道中读取的字符串信息发送给服务端
                        send(client->sockfd, client->recv_buf, sizeof(client->recv_buf), 0);
                    }
                }
            }//for
        }//while
    }
    // 退出进程
    clientClose(client);
}
void clientStart(Client* client)
{

}