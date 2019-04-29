#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<stdlib.h>
#include<errno.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<string.h>

#include "md5.h"

#define ERR_EXIT(m) \
    do { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while (0)

#define MAXLINE 5
#define OPEN_MAX 100
#define LISTENQ 20
#define SERV_PORT 5612
#define INFTIM 1000

#define BUF_LENGHT 1024

#define FILE_NAME "test.png"

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        printf("eg:%s server_ip\n",argv[0]);
        exit(0);
    }

    int sock;
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        //  listenfd = socket(AF_INET, SOCK_STREAM, 0)
        ERR_EXIT("socket error");
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
        ERR_EXIT("connect error");
    }

    struct sockaddr_in localaddr;
    char cli_ip[20];
    socklen_t local_len = sizeof(localaddr);
    memset(&localaddr, 0, sizeof(localaddr));

    if( getsockname(sock,(struct sockaddr *)&localaddr,&local_len) != 0 ){
        ERR_EXIT("getsockname error");
    }
    inet_ntop(AF_INET, &localaddr.sin_addr, cli_ip, sizeof(cli_ip));
    printf("host %s:%d\n", cli_ip, ntohs(localaddr.sin_port)); 

    fd_set rset;
    FD_ZERO(&rset);
    int nready;
    int maxfd;
    /*
    int fd_stdin = fileno(stdin); //
    if (fd_stdin > sock)
        maxfd = fd_stdin;
    else
        maxfd = sock;
    */
    maxfd = sock;
    //uint8_t sendbuf[1024] = {0};
    uint8_t recvbuf[1024] = {0};

    MY_MD5_CTX ctx;
    unsigned char md[16] = {0};
    MD5Init(&ctx);   //初始化一个MD5_CTX这样的结构体
    
    remove(FILE_NAME);
    FILE* sysupdate = fopen(FILE_NAME,"wb+");
    if(sysupdate == NULL){
        printf("open error\n");
        exit(0);
    }
    
    struct timeval tv;

    write(sock, "start", 5);

    while (1)
    {
        //FD_SET(fd_stdin, &rset);
        FD_SET(sock, &rset);

        tv.tv_sec = 0;
        tv.tv_usec = 1000;
        nready = select(maxfd + 1, &rset, NULL, NULL, &tv); //select返回表示检测到可读事件
        if (nready == -1)
            ERR_EXIT("select error");

        if (nready == 0){   
        }
            

        if (FD_ISSET(sock, &rset))
        {

            int ret = read(sock, recvbuf, sizeof(recvbuf)); 
            if (ret == -1)
                ERR_EXIT("read error");
            else if (ret  == 0)   //服务器关闭
            {           
                printf("server close\n");
                MD5Final(md,&ctx);

                printf("MD5: ");
                for(int i = 0; i< 16; i++)
                    printf("%02x",md[i]);
                printf("\n");

                break;
            }

            printf("recv %d bytes\n",ret);
            if(sysupdate != NULL)
            {
                fwrite(recvbuf,sizeof(uint8_t),ret,sysupdate); 
                MD5Update(&ctx,(uint8_t*)recvbuf,ret);
                //write(sock, "ok", 2);
            }

            //write(sock, "ok", 2);
            //memset(recvbuf, 0, sizeof(recvbuf));
        }
            
    }

    fclose(sysupdate);
    close(sock);
    return 0;
}