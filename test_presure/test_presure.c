#include<stdlib.h>
#include<stdio.h>
#include<assert.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/epoll.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>

//每个客户端不停的向服务器发送这个请求
static const char* request="GET http://localhost/judge.html HTTP/1.1\r\nConnection:keep-alive\r\n\r\nxxxxxxxxxxxxx";

#define TRUE 1
#define FALSE 0

//设置非阻塞
int setnonblocking(int fd)
{
    int old_option=fcntl(fd,F_GETFL);
    int new_option=old_option|O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}

void addfd(int epoll_fd,int fd)
{
    struct epoll_event event;
    event.data.fd=fd;
    event.events=EPOLLOUT|EPOLLET|EPOLLERR;
    epoll_ctl(epoll_fd,EPOLL_CTL_ADD,fd,&event);
}

//向服务器写入len字节数据
int write_nbytes(int sockfd,const char* buffer,int len)
{
    int bytes_write=0;
    printf("write out %d bytes to socket %d\n",len,sockfd);
    while(1)
    {
        bytes_write=send(sockfd,buffer,len,0);
        if(bytes_write==-1)
            return FALSE;
        else if(bytes_write==0)
            return FALSE;
	//当写缓冲区太小或满的时候,有可能发不出去
	//要重复发
        len-=bytes_write;
        buffer=buffer+bytes_write;
        if(len<=0)
            return TRUE;
    }
}

//从服务器读取数据
int read_once(int sockfd,char* buffer,int len)
{
    int bytes_read=0;
    memset(buffer,'\0',len);
    bytes_read=recv(sockfd,buffer,len,0);
    if(bytes_read==-1)
        return FALSE;
    else if(bytes_read==0)
        return FALSE;
    printf("read in %d bytes from socket %d with conten:%s\n",bytes_read,sockfd,buffer);
    return TRUE;
}
//向服务器发起num个tcp连接,可以通过改变num来调整测试压力
void start_conn(int epoll_fd,int num,const char *ip,int port)
{
    int ret=0;
    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family=AF_INET;
    address.sin_port=htons(port);
    inet_pton(AF_INET,ip,&address.sin_addr);
    for(int i=0;i<num;++i)
    {
        //sleep(1);
        int sockfd=socket(PF_INET,SOCK_STREAM,0);
        printf("create 1 sock\n");
        if(sockfd<0)
            continue;
        if(connect(sockfd,(struct sockaddr*)&address,sizeof(address))==0)
        {
            printf("build connection %d\n",i);
            addfd(epoll_fd,sockfd);
        }
    }
}
void close_conn(int epollfd,int sockfd)
{
    epoll_ctl(epollfd,EPOLL_CTL_DEL,sockfd,0);
    close(sockfd);
}
int main(int argc,char *argv[])
{
    assert(argc==4);
    int epoll_fd=epoll_create(100);
    start_conn(epoll_fd,atoi(argv[3]),argv[1],atoi(argv[2]));
    struct epoll_event events[10000];
    char buffer[2048];
    while(1)
    {
        int fds=epoll_wait(epoll_fd,events,10000,2000);
        for(int i=0;i<fds;i++)
        {
            int sockfd=events[i].data.fd;
            if(events[i].events&EPOLLIN)
            {
                if(!read_once(sockfd,buffer,2048))
                    close_conn(epoll_fd,sockfd);
                struct epoll_event event;
                event.events=EPOLLOUT|EPOLLET|EPOLLERR;
                event.data.fd=sockfd;
                epoll_ctl(epoll_fd,EPOLL_CTL_MOD,sockfd,&event);
            }
            else if(events[i].events&EPOLLOUT)
            {
                if(!write_nbytes(sockfd,request,strlen(request)))
                    close_conn(epoll_fd,sockfd);
                struct epoll_event event;
                event.events=EPOLLIN|EPOLLET|EPOLLERR;
                event.data.fd=sockfd;
                epoll_ctl(epoll_fd,EPOLL_CTL_MOD,sockfd,&event);
            }
            else if(events[i].events&EPOLLERR)
                close_conn(epoll_fd,sockfd);
        }
    }
}
     

