/******************************************************************************
  A simple program of transfer or receive implementation.
 ******************************************************************************
    Modification:  2018-7
******************************************************************************/
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include "libar8020.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
    
    
typedef struct TX_PARAM{
    
    PORT port;
    int txfile;
    int stop;

}TX_PARAM;

typedef struct RX_PARAM{
    
    PORT port;
    int rxfile;
    int stop;

}RX_PARAM;

typedef struct PKG_PARAM{
    
    PORT port;
    int txfile;
    int rxfile0;
    int rxfile1;
    int rxfile2;
    int stop;

}PKG_PARAM;

/******************************************************************************
* function : show usage
******************************************************************************/
void Sample_TRx_Usage(void)
{
    printf("\nSingle channel sending eg:\n");
    printf("\t\t./sample_transceiver -t file\n\n");

    printf("Double channel sending eg:\n");
    printf("\t\t./sample_transceiver -t file0 file1\n\n");
    
    printf("Pkg sending eg:\n");
    printf("\t\t./sample_transceiver -t file0 file1 file2\n\n");
    
    printf("Single channel receiving eg:\n");
    printf("\t\t./sample_transceiver -r file\n\n");
    
    printf("Double channel receiving eg:\n");
    printf("\t\t./sample_transceiver -r file0 file1\n\n");
    
    printf("Pkg receiving eg:\n");
    printf("\t\t./sample_transceiver -r file0 file1 file3\n\n");

    return;
}

void Sample_Index_Usage(void)
{
    printf("please choose the case which you want to run:\n");
    printf("\t 0) Use channel 0 for transmission.\n");
    printf("\t 1) Use channel 1 for transmission.\n");
    printf("\t 2) Channel 0 and channel 1 are sent simultaneously.\n");
    printf("\t 3) Use channel 0 for receive.\n");
    printf("\t 4) Use channel 1 for receive.\n");
    printf("\t 5) Channel 0 and channel 1 are received at the same time.\n");
    printf("\t 6) pkg sending mode.\n");
    printf("\t 7) pkg receiving mode.\n");
    printf("sample index:");
    
    return;
}

/******************************************************************************
* function : to process abnormal case
******************************************************************************/
void Sample_TRx_Handlesig(int signo)
{
    if (SIGINT == signo || SIGTERM == signo)
    {
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}

/******************************************************************************
* function : to process stream sending
******************************************************************************/
void *Thread_Chn0_Tx(void *p)
{
    TX_PARAM *param = (TX_PARAM *)p;
    int r_cnt,w_cnt;
    int ret;
    char buf0[40960];

    printf("channel 0 send start\n");
    
    while(!param->stop)
    {
        r_cnt = read(param->txfile,buf0,10240);
        if(r_cnt > 0)
        {
            w_cnt = 0;
            do{
                
                ret = Video_Port_Send(param->port,&buf0[w_cnt],r_cnt - w_cnt);
    
                if(ret > 0)
                    w_cnt += ret;
                else
                    usleep(1000);

            }while(w_cnt < r_cnt);
                        
            usleep(33000);

        }
        else if(r_cnt == 0)
        {
             usleep(2000000);
             param->stop = 1;
             printf("end of file\n");
            //lseek(tx_video, 0, SEEK_SET);
        }
    }

    return NULL;
}

void *Thread_Chn1_Tx(void *p)
{
    TX_PARAM *param = (TX_PARAM *)p;
    int r_cnt,w_cnt;
    int ret;
    char buf1[40960];
    
    printf("channel 1 send start\n");
    
    while(!param->stop)
    {
        r_cnt = read(param->txfile,buf1,10240);
        if(r_cnt > 0)
        {
            w_cnt = 0;
            do{
                
                ret = Audio_Port_Send(param->port,&buf1[w_cnt],r_cnt - w_cnt);
    
                if(ret > 0)
                    w_cnt += ret;
                else
                    usleep(1000);

            }while(w_cnt < r_cnt);
                        
            usleep(33000);

        }
        else if(r_cnt == 0)
        {
            usleep(2000000);
            param->stop = 1;
            printf("end of file\n");
            //lseek(tx_video, 0, SEEK_SET);
        }
    }

    return NULL;
}

/******************************************************************************
* function : to process stream receiving
******************************************************************************/
void *Thread_Chn0_Rx(void *p)
{
    RX_PARAM *param = (RX_PARAM *)p;
    int r_cnt,w_cnt;
    int ret;
    char buf0[409600];

    printf("channel 0 send start\n");
    
    while(!param->stop)
    {
        r_cnt = Video_Port_Rec(param->port,buf0,204800);
        if(r_cnt > 0)
        {
            //Storing files is just for demonstration. 
            //Users should use their own decoder instead of saving files.
            w_cnt = 0;
            do{
                
                ret = write(param->rxfile,&buf0[w_cnt],r_cnt - w_cnt);
    
                if(ret > 0)
                    w_cnt += ret;
                else
                    usleep(1000);

            }while(w_cnt < r_cnt);
            
            //usleep(20000);

        }
    }

    return NULL;
}

void *Thread_Chn1_Rx(void *p)
{
    RX_PARAM *param = (RX_PARAM *)p;
    int r_cnt,w_cnt;
    int ret;
    char buf1[409600];
    
    while(!param->stop)
    {
        r_cnt = Audio_Port_Rec(param->port,buf1,204800);
        if(r_cnt > 0)
        {
            //Storing files is just for demonstration. 
            //Users should use their own decoder instead of saving files.
            w_cnt = 0;
            do{
                
                ret = write(param->rxfile,&buf1[w_cnt],r_cnt - w_cnt);
    
                if(ret > 0)
                    w_cnt += ret;
                else
                    usleep(1000);

            }while(w_cnt < r_cnt);
                        
            //usleep(20000);

        }

    }

    return NULL;
}

/******************************************************************************
* function : to process pkg sending
******************************************************************************/
void *Thread_Pkg0_Tx(void *p)
{
    PKG_PARAM *param = (PKG_PARAM *)p;
    int r_cnt,w_cnt;
    int ret;
    char buf0[40960];

    printf("pkg 0 send start\n");
    
    //User defined data
    buf0[0] = 0;
    buf0[1] = 0;
    buf0[2] = 0;
    buf0[3] = 0;
    
    while(!param->stop)
    {
        r_cnt = read(param->txfile,&buf0[4],10240);
        if(r_cnt > 0)
        {
            ret = Pkg_Send(param->port,buf0,r_cnt + 4);    
            usleep(33000);

        }
        else if(r_cnt == 0)
        {
             usleep(2000000);
             param->stop = 1;
             printf("end of file\n");
            //lseek(tx_video, 0, SEEK_SET);
        }
    }

    return NULL;
}

void *Thread_Pkg1_Tx(void *p)
{
    PKG_PARAM *param = (PKG_PARAM *)p;
    int r_cnt,w_cnt;
    int ret;
    char buf0[40960];

    printf("pkg 1 send start\n");
    
    //User defined data
    buf0[0] = 1;
    buf0[1] = 1;
    buf0[2] = 1;
    buf0[3] = 1;
    
    while(!param->stop)
    {
        r_cnt = read(param->txfile,&buf0[4],10240);
        if(r_cnt > 0)
        {
            ret = Pkg_Send(param->port,buf0,r_cnt + 4);    
            usleep(33000);

        }
        else if(r_cnt == 0)
        {
            usleep(2000000);
             param->stop = 1;
             printf("end of file\n");
            //lseek(tx_video, 0, SEEK_SET);
        }
    }

    return NULL;
}

void *Thread_Pkg2_Tx(void *p)
{
    PKG_PARAM *param = (PKG_PARAM *)p;
    int r_cnt,w_cnt;
    int ret;
    char buf0[40960];

    printf("pkg 2 send start\n");
    
    //User defined data
    buf0[0] = 2;
    buf0[1] = 2;
    buf0[2] = 2;
    buf0[3] = 2;
    
    while(!param->stop)
    {
        r_cnt = read(param->txfile,&buf0[4],5120);
        if(r_cnt > 0)
        {
            ret = Pkg_Send(param->port,buf0,r_cnt + 4);    
            usleep(33000);

        }
        else if(r_cnt == 0)
         {
            usleep(2000000);
             param->stop = 1;
             printf("end of file\n");
            //lseek(tx_video, 0, SEEK_SET);
         }
     }

    return NULL;
}


/******************************************************************************
* function : to process pkg receiving
******************************************************************************/
void *Thread_Pkg_Rx(void *p)
{
    PKG_PARAM *param = (PKG_PARAM *)p;
    int r_cnt,w_cnt;
    int ret;
    char buf[MAX_PKG_LEN];
    
    printf("pkg rec start\n");
    
    while(!param->stop)
    {
        r_cnt = Pkg_Rec(param->port,buf,MAX_PKG_LEN);
        if(r_cnt > 0)
        {
            //Storing files is just for demonstration. 
            //Users should use their own decoder instead of saving files.
            if(buf[0] == 0 && buf[1] == 0 && buf[2] == 0 && buf[3] == 0) // save file0
            {
                
                w_cnt = 4;
                do{
                
                    ret = write(param->rxfile0,&buf[w_cnt],r_cnt - w_cnt);
                    if(ret > 0)
                        w_cnt += ret;
                    else
                        usleep(1000);
                
                }while(w_cnt < r_cnt);   
                
            }
            else if(buf[0] == 1 && buf[1] == 1 && buf[2] == 1 && buf[3] == 1) // save file1
            {
                w_cnt = 4;
                do{
                
                    ret = write(param->rxfile1,&buf[w_cnt],r_cnt - w_cnt);
                    if(ret > 0)
                        w_cnt += ret;
                    else
                        usleep(1000);
                
                }while(w_cnt < r_cnt);
                
                
            }
            else if(buf[0] == 2 && buf[1] == 2 && buf[2] == 2 && buf[3] == 2) // save file2
            {
                
                w_cnt = 4;
                do{
                
                    ret = write(param->rxfile2,&buf[w_cnt],r_cnt - w_cnt);
                    if(ret > 0)
                        w_cnt += ret;
                    else
                        usleep(1000);
                
                }while(w_cnt < r_cnt);                
                 
            }
        }
    }
    
    return NULL;
}

/******************************************************************************
* function      : Sample_Chn0_Tx
* Description   : Our mode has two channels. 
                  This function selects channel 0 for sending
******************************************************************************/
int Sample_Chn0_Tx(int argc, char* argv[])
{
    PORT port0;
    int txfile;
    int r_cnt;
    int w_cnt;
    int ret;
    char buf0[40960];
    
    if(strcmp(argv[1],"-t")!=0)
    {
        printf("Unspecified send file\n");
        return -1;
    }
    
    if(Usb_Init() < 0)
    {
        printf("usb init err\n");        
        return -1;
    }
    
    txfile = open(argv[2], O_RDWR, S_IRUSR | S_IWUSR);
    Video_Port_Open(&port0,NULL);
    if(port0 <= 0 || txfile <= 0)
    {
        printf("open err\n");
        return -1;            
    }
    
    while(1)
    {
        r_cnt = read(txfile,buf0,10240);
        if(r_cnt > 0)
        {
            w_cnt = 0;
            do{
                
                ret = Video_Port_Send(port0,&buf0[w_cnt],r_cnt - w_cnt);
                if(ret > 0)
                    w_cnt += ret;
                else
                    usleep(1000);

            }while(w_cnt < r_cnt);
                        
            usleep(33000);

        }
        else if(r_cnt == 0)
        {
            usleep(200000);
             printf("end of file\n");
            //lseek(tx_video, 0, SEEK_SET);
            break;
        }
    }
    
    close(txfile);
    Video_Port_Close(port0);
    Usb_Exit();
}

/******************************************************************************
* function      : Sample_Chn1_Tx
* Description   : Our mode has two channels. 
                  This function selects channel 1 for sending
******************************************************************************/
int Sample_Chn1_Tx(int argc, char* argv[])
{

    PORT port1;
    int txfile;
    int r_cnt;
    int w_cnt;
    int ret;
    char buf1[40960];
    
    if(strcmp(argv[1],"-t")!=0)
    {
        printf("Unspecified send file\n");
        return -1;
    }
    
    if(Usb_Init() < 0)
    {
        printf("usb init err\n");        
        return -1;
    }
    
    txfile = open(argv[2], O_RDWR, S_IRUSR | S_IWUSR);
    
    //At the transmitter side,
    //channel 0 and channel 1 use the same physical port.
    //so we use Video_Port_Open.
    Video_Port_Open(&port1,NULL);
    if(port1 <= 0 || txfile <= 0)
    {
        printf("open err\n");
        return -1;
    }
    
    while(1)
    {
        r_cnt = read(txfile,buf1,10240);
        if(r_cnt > 0)
        {
            w_cnt = 0;
            do{
                
                ret = Audio_Port_Send(port1,&buf1[w_cnt],r_cnt - w_cnt);
                if(ret > 0)
                    w_cnt += ret;
                else
                    usleep(1000);

            }while(w_cnt < r_cnt);
                        
            usleep(33000);

        }
        else if(r_cnt == 0)
        {
            usleep(200000);
            printf("end of file\n");
            //lseek(tx_video, 0, SEEK_SET);
            break;
        }
    }
    
    close(txfile);
    Video_Port_Close(port1);
    Usb_Exit();
}

/******************************************************************************
* function      : Sample_Chn0_Adjust_Tx
* Description   : Our mode has two channels. 
                  This function selects channel 0 for sending
******************************************************************************
int Sample_Chn0_Tx(int argc, char* argv[])
{
    PORT port0;
    PORT cmdport;
    int cmdid;
    int txfile;
    int r_cnt;
    int w_cnt;
    int ret;
    int maxlen;
    char buf0[40960];
    char sbuf[512];
    char rbuf[512];
    
    
    if(strcmp(argv[1],"-t")!=0)
    {
        printf("Unspecified send file\n");
        return -1;
    }
    
    if(Usb_Init() < 0)
    {
        printf("usb init err\n");        
        return -1;
    }
    
    txfile = open(argv[2], O_RDWR, S_IRUSR | S_IWUSR);
    Video_Port_Open(&port0,NULL);
    Cmd_Port_Open(&cmdport,NULL);
    
    if(port0 <= 0 || txfile <= 0 || cmdport <= 0)
    {
        printf("open err\n");
        return -1;            
    }
    
    To_Cmd_ByPass_Mode(cmdport);
    while(read(cmdport,rbuf,512) > 0);
    
    cmdid = 0x81;
    sbuf[0] = 0xff;                 //head
    sbuf[1] = 0x5a;                 //head
    sbuf[2] = (char)cmdid;         //cmdid H
    sbuf[3] = (char)(cmdid >> 8);  //cmdid L
    sbuf[4] = 0;                    //reserve
    sbuf[5] = 0;                    //reserve
    sbuf[6] = 0x00;                 //msg_len
    sbuf[7] = 0x00;                 //msg_len
    sbuf[8] = 0x00;                 //checksum
    sbuf[9] = 0;                    //checksum
    Cmd_Bypass_Send(cmdport,sbuf,10);
                
    while(1)
    {
        ret = Cmd_Bypass_Rec((PORT)p,rbuf,512);
        if(ret >= 16)
        {
            if(rbuf[10] == 0)
            {
                maxlen = rbuf[14] << 24 | rbuf[13] << 16 | rbuf[12] << 8 | rbuf[11];
                printf("free len %08x,mcs = %d",len,rbuf[15]);
            }
            else
            {
                printf("data not ready\n");
            }
        }
                
        r_cnt = read(txfile,buf0,10240);
        if(r_cnt > 0)
        {
            w_cnt = 0;
            do{
                
                ret = Video_Port_Send(port0,&buf0[w_cnt],r_cnt - w_cnt);
                if(ret > 0)
                    w_cnt += ret;
                else
                    usleep(1000);

            }while(w_cnt < r_cnt);
                        
            usleep(33300);

        }
        else if(r_cnt == 0)
        {
            usleep(200000);
             printf("end of file\n");
            //lseek(tx_video, 0, SEEK_SET);
            break;
        }
    }
    
    close(txfile);
    Video_Port_Close(port0);
    Usb_Exit();
}


/******************************************************************************
* function      : Sample_Double_Tx
* Description   : Our mode has two channels. 
                  This function selects channel 0 and channel 1 to send simultaneously
******************************************************************************/
int Sample_Double_Tx(int argc, char* argv[])
{
    pthread_t ThreadId0;
    pthread_t ThreadId1;
    TX_PARAM param0;
    TX_PARAM param1;
    PORT port;
    int txfile0;
    int txfile1;
    
    if((strcmp(argv[1],"-t")!=0) || (argc < 4))
    {
        printf("Unspecified send file\n");
        return -1;
    }
    
    if(Usb_Init() < 0)
    {
        printf("usb init err\n");        
        return -1;
    }
    
    txfile0 = open(argv[2], O_RDWR, S_IRUSR | S_IWUSR);
    txfile1 = open(argv[3], O_RDWR, S_IRUSR | S_IWUSR);
    
    //At the transmitter side,
    //channel 0 and channel 1 use the same physical port.
    //so we use Video_Port_Open.
    Video_Port_Open(&port,NULL);
    if(port <= 0 || txfile0 <= 0 || txfile1 <= 0)
    {
        printf("open err\n");
        return -1;            
    }
    
    param0.txfile = txfile0;
    param0.port = port;
    param0.stop = 0;
    //The transmission thread of channel 0
    if(pthread_create(&ThreadId0, NULL, Thread_Chn0_Tx, &param0) != 0)
    {
        goto exit;
    }
    
    //The transmission thread of channel 1
    param1.txfile = txfile1;
    param1.port = port;
    param1.stop = 0;
    if(pthread_create(&ThreadId1, NULL, Thread_Chn1_Tx, &param1) != 0)
    {
        pthread_join(ThreadId0,NULL);
        goto exit;
    }
    
    while(1)
        usleep(1000000);
    
    pthread_join(ThreadId0,NULL);
    pthread_join(ThreadId1,NULL);
    
exit:    
    close(txfile0);
    close(txfile1);
    Video_Port_Close(port);
    Usb_Exit();
    
    return 0;
}


/******************************************************************************
* function      : Sample_Chn0_Rx
* Description   : Our mode has two channels. 
                  This function selects channel 0 for receiving
******************************************************************************/
int Sample_Chn0_Rx(int argc, char* argv[])
{

    PORT port0;
    int rxfile;
    int r_cnt;
    int w_cnt;
    int ret;
    char buf0[204800];
    
    if(strcmp(argv[1],"-r")!=0)
    {
        printf("Unspecified send file\n");
        return -1;
    }
    
    if(Usb_Init() < 0)
    {
        printf("usb init err\n");        
        return -1;
    }
    
    rxfile = open(argv[2], O_RDWR, S_IRUSR | S_IWUSR);
    Video_Port_Open(&port0,NULL);
    if(port0 <= 0 || rxfile <= 0)
    {
        printf("open err\n");
        return -1;            
    }
    
    while(1)
    {
        r_cnt = Video_Port_Rec(port0,buf0,204800);
        if(r_cnt > 0)
        {
            //Storing files is just for demonstration. 
            //Users should use their own decoder instead of saving files.
            w_cnt = 0;
            do{
                
                ret = write(rxfile,&buf0[w_cnt],r_cnt - w_cnt);
                if(ret > 0)
                    w_cnt += ret;
                else
                    usleep(1000);

            }while(w_cnt < r_cnt);

        }
        else if(r_cnt == 0)
        {
            usleep(1000);
        }
    }
    
    close(rxfile);
    Video_Port_Close(port0);
    Usb_Exit();
}

/******************************************************************************
* function      : Sample_Chn1_Rx
* Description   : Our mode has two channels. 
                  This function selects channel 1 for receiving
******************************************************************************/
int Sample_Chn1_Rx(int argc, char* argv[])
{

    PORT port1;
    int rxfile;
    int r_cnt;
    int w_cnt;
    int ret;
    char buf1[204800];
    
    if(strcmp(argv[1],"-r")!=0)
    {
        printf("Unspecified send file\n");
        return -1;
    }
    
    if(Usb_Init() < 0)
    {
        printf("usb init err\n");        
        return -1;
    }
    
    rxfile = open(argv[2], O_RDWR, S_IRUSR | S_IWUSR);
    Audio_Port_Open(&port1,NULL);
    if(port1 <= 0 || rxfile <= 0)
    {
        printf("open err\n");
        return -1;            
    }
    
    while(1)
    {
        r_cnt = Audio_Port_Rec(port1,buf1,204800);
        if(r_cnt > 0)
        {
            //Storing files is just for demonstration. 
            //Users should use their own decoder instead of saving files.
            w_cnt = 0;
            do{
                
                ret = write(rxfile,&buf1[w_cnt],r_cnt - w_cnt);
                if(ret > 0)
                    w_cnt += ret;
                else
                    usleep(1000);

            }while(w_cnt < r_cnt);

        }
        else if(r_cnt == 0)
        {
            usleep(1000);
        }
    }
     
    close(rxfile);
    Audio_Port_Close(port1);
    Usb_Exit();
}


/******************************************************************************
* function      : Sample_Double_Rx
* Description   : Our mode has two channels. 
                  This function selects channel 0 and channel 1 to receive simultaneously
******************************************************************************/
int Sample_Double_Rx(int argc, char* argv[])
{
    int rxfile0;
    int rxfile1;
    PORT port0;
    PORT port1;
    RX_PARAM param0;
    RX_PARAM param1;
    pthread_t ThreadId0;
    pthread_t ThreadId1;
    
    if((strcmp(argv[1],"-r")!=0) || (argc < 4))
    {
        printf("Unspecified send file\n");
        return -1;
    }
    
    if(Usb_Init() < 0)
    {
        printf("usb init err\n");        
        return -1;
    }
    
    rxfile0 = open(argv[2], O_RDWR, S_IRUSR | S_IWUSR);
    rxfile1 = open(argv[3], O_RDWR, S_IRUSR | S_IWUSR);
    Video_Port_Open(&port0,NULL);
    Audio_Port_Open(&port1,NULL);
    if(port0 <= 0 || port1 <= 0 || rxfile0 <= 0 || rxfile1 < 0)
    {
        printf("open err\n");
        return -1;            
    }
    
    param0.rxfile = rxfile0;
    param0.port = port0;
    param0.stop = 0;
    //The transmission thread of channel 0
    if(pthread_create(&ThreadId0, NULL, Thread_Chn0_Rx, &param0) != 0)
    {
        goto exit;
    }
    
    //The transmission thread of channel 1
    param1.rxfile = rxfile1;
    param1.port = port1;
    param1.stop = 0;
     if(pthread_create(&ThreadId1, NULL, Thread_Chn1_Rx, &param1) != 0)
    {
        pthread_join(ThreadId0,NULL);
        goto exit;
    }
     
    while(1)
        usleep(1000000);
    
    pthread_join(ThreadId0,NULL);
    pthread_join(ThreadId1,NULL);

exit:    
    close(rxfile0);
    close(rxfile1);
    Video_Port_Close(port0);
    Audio_Port_Close(port1);
    Usb_Exit();
}

/******************************************************************************
* function      : Sample_Pkg_Tx
* Description   : By sending packets, users can send multi-channel data at the same time.
******************************************************************************/
int Sample_Pkg_Tx(int argc, char* argv[])
{

    PORT port;
    
    PKG_PARAM param0;
    PKG_PARAM param1;
    PKG_PARAM param2;
    
    pthread_t ThreadId0;
    pthread_t ThreadId1;
    pthread_t ThreadId2;
    
    if((strcmp(argv[1],"-t")!=0) || (argc < 5))
    {
        printf("Unspecified send file\n");
        return -1;
    }
    
    if(Usb_Init() < 0)
    {
        printf("usb init err\n");
        return -1;
    }
    
    param0.txfile = open(argv[2], O_RDWR, S_IRUSR | S_IWUSR);
    param1.txfile = open(argv[3], O_RDWR, S_IRUSR | S_IWUSR);
    param2.txfile = open(argv[4], O_RDWR, S_IRUSR | S_IWUSR);
    
    Pkg_Open(&port,NULL);
    if(port <= 0 || param0.txfile <= 0 || param1.txfile <= 0 || param2.txfile <= 0)
    {
        printf("open err\n");
        return -1;            
    }
    
    //The transmission thread of pkg 0
    param0.port = port;
    param0.stop = 0;
    if(pthread_create(&ThreadId0, NULL, Thread_Pkg0_Tx, &param0) != 0)
    {
        goto exit;
    }
 
    //The transmission thread of pkg 1
    param1.port = port;
    param1.stop = 0;
    if(pthread_create(&ThreadId1, NULL, Thread_Pkg1_Tx, &param1) != 0)
    {
        pthread_join(ThreadId0,NULL);
        goto exit;
    }
    
    //The transmission thread of pkg 2
    param2.port = port;
    param2.stop = 0;
    if(pthread_create(&ThreadId2, NULL, Thread_Pkg2_Tx, &param2) != 0)
    {
        pthread_join(ThreadId0,NULL);
        pthread_join(ThreadId1,NULL);
        goto exit;
    }
   
    while(1)
        usleep(1000000);
    
    pthread_join(ThreadId0,NULL);
    pthread_join(ThreadId1,NULL);
    pthread_join(ThreadId2,NULL);
        
exit:
    close(param0.txfile);
    close(param1.txfile);
    close(param2.txfile);
    Pkg_Close(port);
    Usb_Exit();
}

/******************************************************************************
* function      : Sample_Pkg_Rx
* Description   : By receiving packets, users can receive multi-channel data at the same time.
******************************************************************************/
int Sample_Pkg_Rx(int argc, char* argv[])
{
    PORT port;
    PKG_PARAM param;
    pthread_t ThreadId;
    
    if((strcmp(argv[1],"-r")!=0) || (argc < 5))
    {
        printf("Unspecified send file\n");
        return -1;
    }
    
    if(Usb_Init() < 0)
    {
        printf("usb init err\n");        
        return -1;
    }
    
    param.rxfile0 = open(argv[2], O_RDWR, S_IRUSR | S_IWUSR);
    param.rxfile1 = open(argv[3], O_RDWR, S_IRUSR | S_IWUSR);
    param.rxfile2 = open(argv[4], O_RDWR, S_IRUSR | S_IWUSR);
    Pkg_Open(&port,NULL);
    
    if(port <= 0 || param.rxfile0 <= 0 || param.rxfile1 <= 0 || param.rxfile2 <= 0)
    {
        printf("open err\n");
        return -1;            
    }
    
    param.port = port;
    param.stop = 0;
    //The receive thread of pkg
    if(pthread_create(&ThreadId, NULL, Thread_Pkg_Rx, &param) != 0)
    {
        goto exit;
    }
    
    while(1)
        usleep(1000000);
    
    pthread_join(ThreadId,NULL);
        
exit:
    close(param.rxfile0);
    close(param.rxfile1);
    close(param.rxfile2);
    Pkg_Close(port);
    Usb_Exit();
}


/******************************************************************************
* function    : main()
* Description : data transfer and receive
******************************************************************************/

int main(int argc, char* argv[])
{
    int ret;
    char ch;
    
    if (argc < 3 || ((strcmp(argv[1],"-t")!=0) && (strcmp(argv[1],"-r")!=0)))
    {
        Sample_TRx_Usage();
        return -1;
    }

//    signal(SIGINT, Sample_TRx_Handlesig);
//    signal(SIGTERM, Sample_TRx_Handlesig);
    
    Sample_Index_Usage();
    ch = (char)getchar();
    getchar();
    switch (ch)
    {

//case '0' ~ '5' is only valid in stream mode
        
        case '0':/* Use channel 0 for transmission */
            ret = Sample_Chn0_Tx(argc,argv);
            break;
        case '1':/* Use channel 1 for transmission */
            ret = Sample_Chn1_Tx(argc,argv);
            break;
        case '2':/* Channel 0 and channel 1 can be sent at the same time */
            ret = Sample_Double_Tx(argc,argv);
            break;
        case '3':/* Use channel 0 for receive */
            ret = Sample_Chn0_Rx(argc,argv);
            break;
        case '4':/* Use channel 1 for receive */
            ret = Sample_Chn1_Rx(argc,argv);
            break;
        case '5':/* Channel 0 and channel 1 can be received at the same time */
            ret = Sample_Double_Rx(argc,argv);
            break;
            
//case '6' ~ '7' is only valid in pkg mode            
            
        case '6':/* pkg sending mode */
            ret = Sample_Pkg_Tx(argc,argv);
            break;
        case '7':/* pkg receiving mode */
            ret = Sample_Pkg_Rx(argc,argv);
            break;            
             
        default:
            printf("the index is invaild!\n");
            Sample_Index_Usage();
            return -1;
    }

    if (0 == ret)
    { printf("program exit normally!\n"); }
    else
    { printf("program exit abnormally!\n"); }
    exit(ret);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
