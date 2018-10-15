#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <pthread.h>

#include "my_socket_s.h"
#include "com.h"


#define MAX_EVENT_NUMBER 1024
#define UDP_BUFFER_SIZE 1024
#define UDP_PORT 1234

static int setnonblocking( int fd )
{
    int old_option = fcntl( fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;
    fcntl( fd, F_SETFL, new_option );
    return old_option;
}

static void addfd( int epollfd, int fd )
{
    epoll_event event;
    event.data.fd = fd;
    //event.events = EPOLLIN | EPOLLET;
    event.events = EPOLLIN;//可读事件，默认为LT模式，事件一般被触发多次
    epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event );
    setnonblocking( fd );
}

extern Serial serial; 

void * udp_net(void *arg)
{
    int ret = 0;
    struct sockaddr_in address;
    int udpfd = -1;
    //创建udp套接字，并绑定
    bzero( &address, sizeof( address ) );
    address.sin_family = AF_INET;
    //inet_pton( AF_INET, ip, &address.sin_addr );
    address.sin_port = htons( UDP_PORT );
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    //创建套接口
    if ((udpfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket create failed\n");
        exit(EXIT_FAILURE);
    }
    puts("socket create success");

    ret = bind( udpfd, ( struct sockaddr* )&address, sizeof( address ) );
    assert( ret != -1 );
    puts("bind success");

    epoll_event events[ MAX_EVENT_NUMBER ];
    int epollfd = epoll_create( 5 );
    assert( epollfd != -1 );

    //注册udp套接字的可读事件
    addfd( epollfd, udpfd );
    addfd( epollfd, serial.fd[0] );

    char ReadBuff[128] = {0};
    int ReadLen = sizeof(ReadBuff);
    int len = 0;

    while( 1 )
    {
        int number = epoll_wait( epollfd, events, MAX_EVENT_NUMBER, 1000 );
        if ( number < 0 )
        {
            printf( "epoll failure\n" );
            break;
        }

        for ( int i = 0; i < number; i++ )
        {
            int sockfd = events[i].data.fd;
            if ( sockfd == udpfd )
            {
                char buf[ UDP_BUFFER_SIZE ];
                memset( buf, '\0', UDP_BUFFER_SIZE );
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof( client_address );

                ret = recvfrom( udpfd, buf, UDP_BUFFER_SIZE-1, 0, ( struct sockaddr* )&client_address, &client_addrlength );
                // buf[ret] = 0;
                // printf("%s",buf);
                write(serial.fd[0], buf,ret);
                // if( ret > 0 )
                // {
                //     sendto( udpfd, buf, UDP_BUFFER_SIZE-1, 0, ( struct sockaddr* )&client_address, client_addrlength );
                // }
            }
            //注册的socket发生可读事件
            else if ( events[i].events & EPOLLIN )
            {
                if ( sockfd == serial.fd[0] )
                {
                    len = read(events[i].data.fd, ReadBuff, ReadLen);
                    tcdrain(serial.fd[0]);//等待所有输出都被传输
                    //tcflush(serial.fd[0],TCIOFLUSH);//刷清未决输入和/或输出
                    ReadBuff[len] = 0;
                    printf("uart1: %s\n",ReadBuff);
                    //sendto( udpfd, ReadBuff, len, 0, ( struct sockaddr* )&client_address, client_addrlength );
                    if(len != ReadLen) //如何保证每次都读到这些字节又不阻塞！ 
                    {
                        bzero(ReadBuff,ReadLen);
                    }	
                }   
            }
            else
            {
                printf( "something else happened \n" );
            }
        }
    }
    pthread_exit(NULL);
}