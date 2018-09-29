#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <pthread.h>
#include <sys/wait.h>

#include "libar8020.h"
#include "H264_UVC_TestAP.h"

char stop = 0;
int tx_vedio;

pthread_t ThreadId2;
pthread_t ThreadId3;
const char *dev_name = "/dev/video1";

//RingBuffer* rbuf;

void *Thread_Tx_Vedio(void *p)
{

    int r_cnt,w_cnt;
    int ret;
    char* buf1 = (char *)malloc(15360);

    usleep(2000000);
    printf("vedio send start\n");
    while(!stop)
    {
        //r_cnt = RingBuffer_read(rbuf,buf1,15360);
        r_cnt = read(tx_vedio,buf1,15360);
        if(r_cnt > 0)
        {
            w_cnt = 0;
            do{

                ret = Video_Port_Send((PORT)p,&buf1[w_cnt],r_cnt - w_cnt);
                if(ret > 0)
                    w_cnt += ret;
                else
                    usleep(1000);

                if(stop)
                break;

            }while(w_cnt < r_cnt);

            usleep(33000);
        }
        else if(r_cnt == 0)
        {
            usleep(2000000);
            //stop = 1;
            printf("----------------no buffer--------------\n");
            //lseek(tx_vedio, 0, SEEK_SET);
        }
    }

    pthread_exit(NULL);
}

void usege(char *demo)
{
    printf("please input a H264 video device and set bitrate\n");
    printf("demo:%s device rate\n",demo);
    printf("eg:%s /dev/video1 4000000\n",demo);
}

int main(int argc,char **argv)
{
    PORT port1 = 0;
    PORT port0 = 0;
    int ret;
    WIR_INFO wir_info;

    if(argc < 3)
    {
        usege(argv[0]);
        Init_264camera(dev_name);
        Set_BitRate = 2048*1024;
    }
    else
    {
        Init_264camera(argv[1]);
        Set_BitRate = atoi(argv[2]);
    }
    //tx_vedio = open(argv[1], O_RDWR, S_IRUSR | S_IWUSR);
    // rbuf = RingBuffer_create(DEFAULT_BUF_SIZE);

    Video_Port_Open(&port1,NULL);
    Cmd_Port_Open(&port0,NULL);

    while(port1 <= 0)
    {
        printf("open err ,check artosyn modem!\n");
        sleep(1);
        Vedio_Port_Open(&port1,NULL);	
        Cmd_Port_Open(&port0,NULL);	
    }

    

    if(pthread_create(&ThreadId2, NULL, cap_video, port1) != 0)
    {
        stop = 1;
        usleep(100 * 1000);
        goto exit1;
    }

    // if(pthread_create(&ThreadId3, NULL, Thread_Tx_Vedio, port1) != 0)
    // {
    //     stop = 1;
    //     usleep(100 * 1000);
    //     goto exit1;
    // }

    while(!stop)
    {
        ret = Cmd_Get_Info(port0,&wir_info);
        if(ret == 0)
        {
            printf("max_bps : %d\n",wir_info.max_bps);
            printf("match_state : %d\n",wir_info.match_state);
            printf("vedio_space : %d\n",*(int *)(wir_info.vedio_space));
        }
        else if(ret < 0)
            stop = 1;

        usleep(2000000);
    }

    usleep(2000000);

    Video_Port_Close(port1);
    Cmd_Port_Close(port0);
    pthread_join(ThreadId2,NULL);
    // pthread_join(ThreadId3,NULL);
    return 0;

    exit1:
    Video_Port_Close(port1);
    return 0;

}
