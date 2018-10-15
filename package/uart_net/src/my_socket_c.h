#ifndef _MY_SOCKET_C_H_
#define _MY_SOCKET_C_H_

#define MAX_EVENT_NUMBER 1024
#define TCP_BUFFER_SIZE 512
#define UDP_BUFFER_SIZE 1024
#define UDP_PORT 1234

int setnonblocking( int fd );
void addfd( int epollfd, int fd );
void * net_handle( const char* ip, int port );

#endif