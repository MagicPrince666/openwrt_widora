#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <pthread.h>
#include <sys/wait.h>
#include <errno.h>  
#include <string.h>  
#include "libar8020.h"

char stop = 0;

pthread_t ThreadId0;

void Stop(int signo)
{
    stop = 1;
    _exit(0);
}

unsigned int data_check(char *buffer,int len)
{
    int i;
    unsigned int sum = 0;
    for(i = 0;i < len;i ++)
        sum += (unsigned char)buffer[i];
    
    return sum;
}

void print_buf(char *buf,int len)
{
    int i;
    for(i = 0;i < len;i++)
        printf("%02x ",buf[i]);
    printf("\n");
}

void print_msg(void)
{
    printf("\nplease input a,b,c,d,e,f,g, then 'enter' :\n \
    'a' : get video free buf len and mcs value\n \
    'b' : get grd c201-d module status info\n \
    'c' : get sky c201-d module status info\n \
    'd' : write usb bypass demo data\n \
    'e' : read usb bypass demo data\n \
    'f' : write uart5 bypass demo data\n \
    'g' : read uart5 bypass demo data\n \
    'h' : debug info from mode\n");
   
}
void *Thread_Cmd(void *p)
{
    int ret;
    int cmdid;
    int i;
    char a;
    char sbuf[512];
    char rbuf[512];
	int len;
    unsigned int sum_check;
    unsigned int sendcnt1,sendcnt2,reccnt1,reccnt2;

    To_Cmd_ByPass_Mode((PORT)p);
    usleep(100000);
    
    while(!stop)
    {
        print_msg();
        a = getchar();
        getchar();
        
        switch(a)
        {
            case 'a':   //this is the way to get video buffer length
            
                while(read((int)p,rbuf,512) > 0);
                
                //usr pkg
                cmdid = 0x81;
                sbuf[0] = 0xff;                 //head
                sbuf[1] = 0x5a;                 //head
                sbuf[2] = (char)cmdid;          //cmdid H
                sbuf[3] = (char)(cmdid >> 8);   //cmdid L
                sbuf[4] = 0;                    //reserve
                sbuf[5] = 0;                    //reserve
                sbuf[6] = 0x00;                 //msg_len
                sbuf[7] = 0x00;                 //msg_len
                sbuf[8] = 0x00;                 //checksum
                sbuf[9] = 0;                    //checksum

                //send to mode
                Cmd_Bypass_Send((PORT)p,sbuf,10);
                usleep(20000);
                //get info from mode
                ret = Cmd_Bypass_Rec((PORT)p,rbuf,512);
                if(ret >= 16)
                {
                    if(rbuf[10] == 0)
                    {
                        len = rbuf[14] << 24 | rbuf[13] << 16 | rbuf[12] << 8 | rbuf[11]; 
                        printf("free len %08x,mcs = %d",len,rbuf[15]);
                    }
                    else
                    {
                        printf("data not ready\n");
                    }
                }
                else
                {
                    printf("get video buffer len error %d",ret);
                }
                
                break;
            
		
            case 'b':
            
                while(read((int)p,rbuf,512) > 0);
                
                //usr pkg
                cmdid = 0x82;
                sbuf[0] = 0xff;                 //head
                sbuf[1] = 0x5a;                 //head
                sbuf[2] = (char)cmdid;          //cmdid H
                sbuf[3] = (char)(cmdid >> 8);   //cmdid L
                sbuf[4] = 0;                    //reserve
                sbuf[5] = 0;                    //reserve
                sbuf[6] = 0x00;                 //msg_len
                sbuf[7] = 0x00;                 //msg_len
                sbuf[8] = 0x00;                 //checksum
                sbuf[9] = 0;                    //checksum

                //send to mode
                Cmd_Bypass_Send((PORT)p,sbuf,10);
                usleep(20000);
                //get info from mode
                ret = Cmd_Bypass_Rec((PORT)p,rbuf,512);
                if(ret >= 21)
                {
                    if(rbuf[10] == 0)
                    {
                        printf("lock status: %s, video status: %s\n",(rbuf[11] == 0)?"unlock":((rbuf[11] == 1) ? "lock":"searchId"),
                        (rbuf[12] == 0)?"noVideoData":((rbuf[12] == 1) ? "VideoOk":"VideoFull"));
                        printf("grd sigQal %d, sky sigQal %d\n",rbuf[13],rbuf[14]);
                        printf("grd rssi A:%d,B:%d, sky rssi A:%d,B:%d\n",rbuf[15],rbuf[16],rbuf[17],rbuf[18]);
                        printf("grd errRate %d, sky errRate %d\n",rbuf[19],rbuf[20]);
                    }
                    else
                    {
                        printf("data not ready\n");
                    }
                }
                else
                {
                    printf("get grd mod status len error %d",ret);
                }
                break;
		
            case 'c':

                while(read((int)p,rbuf,512) > 0);
                
                //usr pkg
                cmdid = 0x83;
                sbuf[0] = 0xff;                 //head
                sbuf[1] = 0x5a;                 //head
                sbuf[2] = (char)cmdid;          //cmdid H
                sbuf[3] = (char)(cmdid >> 8);   //cmdid L
                sbuf[4] = 0;                    //reserve
                sbuf[5] = 0;                    //reserve
                sbuf[6] = 0x00;                 //msg_len
                sbuf[7] = 0x00;                 //msg_len
                sbuf[8] = 0x00;                 //checksum
                sbuf[9] = 0;                    //checksum

                //send to mode
                Cmd_Bypass_Send((PORT)p,sbuf,10);
                usleep(20000);
                //get info from mode
                ret = Cmd_Bypass_Rec((PORT)p,rbuf,512);
                if(ret >= 17)
                {
                    if(rbuf[10] == 0)
                    {
                        printf("lock status: %s, video status: %s\n",(rbuf[11] == 0)?"unlock":((rbuf[11] == 1) ? "lock":"searchId"),
                        (rbuf[12] == 0)?"noVideoData":((rbuf[12] == 1) ? "VideoOk":"VideoFull"));
                        printf("sky sigQal %d\n",rbuf[13]);
                        printf("sky rssi A:%d,B:%d\n",rbuf[14],rbuf[15]);
                        printf("sky errRate %d\n",rbuf[16]);
                    }
                    else
                    {
                        printf("data not ready\n");
                    }
                }
                else
                {
                    printf("get sky mod status len error %d",ret);
                }
                
                break;
                
            case 'd':

                while(read((int)p,rbuf,512) > 0);
                
                //usr pkg
                cmdid = 0x84;
                sbuf[0] = 0xff;                 //head
                sbuf[1] = 0x5a;                 //head
                sbuf[2] = (char)cmdid;          //cmdid H
                sbuf[3] = (char)(cmdid >> 8);   //cmdid L
                sbuf[4] = 0;                    //reserve
                sbuf[5] = 0;                    //reserve
                
                
                #define DEFAULT_SEND_LEN (10)
                for(i=0;i<DEFAULT_SEND_LEN;i++)
                sbuf[i+10] = i;
                sum_check = data_check(sbuf+10,DEFAULT_SEND_LEN);
                sbuf[6] = DEFAULT_SEND_LEN;     //msg_len
                sbuf[7] = 0x00;                 //msg_len
                sbuf[8] = sum_check;            //checksum
                sbuf[9] = sum_check >> 8;       //checksum
                
                //send to mode
                ret = Cmd_Bypass_Send((PORT)p,sbuf,10+DEFAULT_SEND_LEN);
                
                if(ret < 0)
                {
                    printf("send failed\n");
                }
                else
                {
                    printf("send ok\n");
                }
                usleep(20000);
                break;
		
            case 'e':

                while(read((int)p,rbuf,512) > 0);
                
                //usr pkg
                cmdid = 0x85;
                sbuf[0] = 0xff;                 //head
                sbuf[1] = 0x5a;                 //head
                sbuf[2] = (char)cmdid;          //cmdid H
                sbuf[3] = (char)(cmdid >> 8);   //cmdid L
                sbuf[4] = 0;                    //reserve
                sbuf[5] = 0;                    //reserve
                sbuf[6] = 0x00;                 //msg_len
                sbuf[7] = 0x00;                 //msg_len
                sbuf[8] = 0x00;                 //checksum
                sbuf[9] = 0;                    //checksum
                
                //send to mode
                Cmd_Bypass_Send((PORT)p,sbuf,10);
                usleep(20000);
                //get info from mode
                ret = Cmd_Bypass_Rec((PORT)p,rbuf,512);
                if(ret >= 10)
                {
                    printf("usb bypass data :");
                    print_buf(rbuf+10,(ret) > 20 ? 10 : ret-10);
                }
                else
                {
                    printf("get usb len error %d",ret);
                }
                
                break;
		
            case 'f':

                while(read((int)p,rbuf,512) > 0);
                
                //usr pkg
                cmdid = 0x86;
                sbuf[0] = 0xff;                 //head
                sbuf[1] = 0x5a;                 //head
                sbuf[2] = (char)cmdid;          //cmdid H
                sbuf[3] = (char)(cmdid >> 8);   //cmdid L
                sbuf[4] = 0;                    //reserve
                sbuf[5] = 0;                    //reserve
                
                
                #define DEFAULT_SEND_LEN (10)
                for(i=0;i<DEFAULT_SEND_LEN;i++)
                sbuf[i+10] = i;
                sum_check = data_check(sbuf+10,DEFAULT_SEND_LEN);
                sbuf[6] = DEFAULT_SEND_LEN;     //msg_len
                sbuf[7] = 0x00;                 //msg_len
                sbuf[8] = sum_check;            //checksum
                sbuf[9] = sum_check >> 8;       //checksum
                
                //send to mode
                ret = Cmd_Bypass_Send((PORT)p,sbuf,10+DEFAULT_SEND_LEN);
                if(ret < 0)
                {
                    printf("send failed\n");
                }
                else
                {
                    printf("send ok\n");
                }
                usleep(20000);
                
                break;
		
            case 'g':

                while(read((int)p,rbuf,512) > 0);
                
                //usr pkg
                cmdid = 0x87;
                sbuf[0] = 0xff;                 //head
                sbuf[1] = 0x5a;                 //head
                sbuf[2] = (char)cmdid;          //cmdid H
                sbuf[3] = (char)(cmdid >> 8);   //cmdid L
                sbuf[4] = 0;                    //reserve
                sbuf[5] = 0;                    //reserve
                sbuf[6] = 0x00;                 //msg_len
                sbuf[7] = 0x00;                 //msg_len
                sbuf[8] = 0x00;                 //checksum
                sbuf[9] = 0;                    //checksum
                
                //send to mode
                Cmd_Bypass_Send((PORT)p,sbuf,10);
                usleep(20000);
                //get info from mode
                ret = Cmd_Bypass_Rec((PORT)p,rbuf,512);
                if(ret >= 10)
                {
                    printf("usb bypass uart5 data :");
                    print_buf(rbuf+10,(ret) > 20 ? 10 : ret-10);
                }
                else
                {
                    printf("get uart5 len error %d",ret);
                }
                
                break;

            case 'h':

                while(read((int)p,rbuf,512) > 0);
                
                //usr pkg
                cmdid = 0x88;
                sbuf[0] = 0xff;                 //head
                sbuf[1] = 0x5a;                 //head
                sbuf[2] = (char)cmdid;          //cmdid H
                sbuf[3] = (char)(cmdid >> 8);   //cmdid L
                sbuf[4] = 0;                    //reserve
                sbuf[5] = 0;                    //reserve
                sbuf[6] = 0x00;                 //msg_len
                sbuf[7] = 0x00;                 //msg_len
                sbuf[8] = 0x00;                 //checksum
                sbuf[9] = 0;                    //checksum
                
                //send to mode
                Cmd_Bypass_Send((PORT)p,sbuf,10);
                usleep(20000);
                //get info from mode
                ret = Cmd_Bypass_Rec((PORT)p,rbuf,512);
                if(ret >= 10)
                {
                    sendcnt1 = rbuf[11] << 24 | rbuf[12] << 16 | rbuf[13] << 8 | rbuf[14];
                    sendcnt2 = rbuf[15] << 24 | rbuf[16] << 16 | rbuf[17] << 8 | rbuf[18];
                    reccnt1 = rbuf[19] << 24 | rbuf[20] << 16 | rbuf[21] << 8 | rbuf[22];
                    reccnt2 = rbuf[23] << 24 | rbuf[24] << 16 | rbuf[25] << 8 | rbuf[26];
                    
                    printf("debug info from module :");
                    printf("sendcnt1:%d,sendcnt2:%d,reccnt1:%d,reccnt2:%d\n",sendcnt1,sendcnt2,reccnt1,reccnt2);
                    //print_buf(rbuf+10,(ret) > 20 ? 10 : ret-10);
                }
                else
                {
                    printf("get module info err %d",ret);
                }
                
                break;
		
            default :
                printf("error input data\n");
                break;
        }
    }
    
    return NULL;
}


int main()
{
    PORT port0;
    int ret;

    signal(SIGINT,Stop);
    ret = Cmd_Port_Open(&port0,NULL);
    if(ret < 0)
    {
        printf("usb open err %d\n",ret);
        return -1;			
    }
    
    if(pthread_create(&ThreadId0, NULL, Thread_Cmd, port0) != 0)
    {
        printf("thread0 creat err\n");
        return 0;
    }

    while(!stop)
    {
        usleep(1000000);
    }

    Cmd_Port_Close(port0);
    pthread_join(ThreadId0,NULL);

    return 0;
}


