#ifndef WIN32
   #include <unistd.h>
   #include <cstdlib>
   #include <cstring>
   #include <netdb.h>
#else
   #include <winsock2.h>
   #include <ws2tcpip.h>
   #include <wspiapi.h>
#endif
#include <iostream>
#include <udt.h>
#include "cc.h"
#include "test_util.h"
#include <sys/ioctl.h>
#include "v4l2uvc.h"
#include "h264_xu_ctrls.h"
#include "H264_UVC_TestAP.h"

using namespace std;

#ifndef WIN32
void* monitor(void*);
#else
DWORD WINAPI monitor(LPVOID);
#endif

//#define VIDEO_FILE
#define VIDEO_DEV

#ifdef VIDEO_FILE
#define PACKAGE 15360
FILE* g_txvideo = NULL;
#endif

#ifdef VIDEO_DEV
extern struct vdIn *vd;
extern struct buffer *buffers;
#endif


int main(int argc, char* argv[])
{
#ifdef VIDEO_FILE
   if ((4 != argc) || (0 == atoi(argv[2])))
   {
      cout << "usage: appclient server_ip server_port file_name" << endl;
      return 0;
   }
#endif

#ifdef VIDEO_DEV
   if ((3 != argc) || (0 == atoi(argv[2])))
   {
      cout << "usage: appclient server_ip server_port" << endl;
      return 0;
   }
#endif
   // Automatically start up and clean up UDT module.
   UDTUpDown _udt_;

   struct addrinfo hints, *local, *peer;

   memset(&hints, 0, sizeof(struct addrinfo));

   hints.ai_flags = AI_PASSIVE;
   hints.ai_family = AF_INET;
   hints.ai_socktype = SOCK_STREAM;
   //hints.ai_socktype = SOCK_DGRAM;

   if (0 != getaddrinfo(NULL, "9000", &hints, &local))
   {
      cout << "incorrect network address.\n" << endl;
      return 0;
   }

   UDTSOCKET client = UDT::socket(local->ai_family, local->ai_socktype, local->ai_protocol);

   // UDT Options
   //UDT::setsockopt(client, 0, UDT_CC, new CCCFactory<CUDPBlast>, sizeof(CCCFactory<CUDPBlast>));
   //UDT::setsockopt(client, 0, UDT_MSS, new int(9000), sizeof(int));
   //UDT::setsockopt(client, 0, UDT_SNDBUF, new int(10000000), sizeof(int));
   //UDT::setsockopt(client, 0, UDP_SNDBUF, new int(10000000), sizeof(int));
   //UDT::setsockopt(client, 0, UDT_MAXBW, new int64_t(12500000), sizeof(int));

   // Windows UDP issue
   // For better performance, modify HKLM\System\CurrentControlSet\Services\Afd\Parameters\FastSendDatagramThreshold
   #ifdef WIN32
      UDT::setsockopt(client, 0, UDT_MSS, new int(1052), sizeof(int));
   #endif

   // for rendezvous connection, enable the code below
   /*
   UDT::setsockopt(client, 0, UDT_RENDEZVOUS, new bool(true), sizeof(bool));
   if (UDT::ERROR == UDT::bind(client, local->ai_addr, local->ai_addrlen))
   {
      cout << "bind: " << UDT::getlasterror().getErrorMessage() << endl;
      return 0;
   }
   */

   freeaddrinfo(local);

   if (0 != getaddrinfo(argv[1], argv[2], &hints, &peer))
   {
      cout << "incorrect server/peer address. " << argv[1] << ":" << argv[2] << endl;
      return 0;
   }

   // connect to the server, implict bind
   if (UDT::ERROR == UDT::connect(client, peer->ai_addr, peer->ai_addrlen))
   {
      cout << "connect: " << UDT::getlasterror().getErrorMessage() << endl;
      return 0;
   }

   freeaddrinfo(peer);

#ifdef VIDEO_FILE
      g_txvideo = fopen(argv[3], "rb");
	if(g_txvideo == NULL)
	{
		printf("can\'t open %s !\n",argv[3]);
		fclose(g_txvideo);
    	      g_txvideo = NULL; /* 需要指向空，否则会指向原打开文件地址 */
		return 1;
	}

	printf("Transmit %s !\n",argv[3]);


   // using CC method
   //CUDPBlast* cchandle = NULL;
   //int temp;
   //UDT::getsockopt(client, 0, UDT_CC, &cchandle, &temp);
   //if (NULL != cchandle)
   //   cchandle->setRate(500);

   //int size = 100000;
   int size = PACKAGE;
   char* data = new char[size];
#endif

   #ifndef WIN32
      pthread_create(new pthread_t, NULL, monitor, &client);
   #else
      CreateThread(NULL, 0, monitor, &client, 0, NULL);
   #endif

#ifdef VIDEO_DEV
      int ret;
	struct v4l2_buffer buf0;
	Init_264camera();

	struct timeval tv;
      tv.tv_sec = 0;
      tv.tv_usec = 10000;
	fd_set rfds;
      int retval=0;

      char* data = NULL;
      int r_cnt,w_cnt;
	for(;;)
      {
            memset (&buf0, 0, sizeof (buf0));
                        
            buf0.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf0.memory = V4L2_MEMORY_MMAP;

            FD_ZERO(&rfds);
            FD_SET(vd->fd, &rfds);
            
            retval=select(vd->fd + 1, &rfds, NULL, NULL, &tv);
            if(retval<0)
            {  
                  perror("select error\n");  
            }
            else//有数据要收
            {		
                  ret = ioctl(vd->fd, VIDIOC_DQBUF, &buf0);
                  if (ret < 0) 
                  {
                        printf("Unable to dequeue buffer!\n");
                        exit(1);
                  }	  
                  
                  r_cnt = buf0.bytesused;
                  data = (char*)(buffers[buf0.index].start);
                  if(r_cnt > 0)
                  {
                        w_cnt = 0;
                        do{
                              if (UDT::ERROR == (ret = UDT::send(client, data + w_cnt, r_cnt - w_cnt, 0)))
                              {
                                    cout << "send:" << UDT::getlasterror().getErrorMessage() << endl;
                                    break;
                              }
                              if(ret > 0)
                              w_cnt += ret;
                              else
                              usleep(1000);
                              
                        }while(w_cnt < r_cnt);

                        //usleep(33000);
                  }
                  // fwrite(buffers[buf.index].start, buf.bytesused, 1, rec_fp1);
                  


                  ret = ioctl(vd->fd, VIDIOC_QBUF, &buf0);
                  
                  if (ret < 0) 
                  {
                        printf("Unable to requeue buffer");
                        exit(1);
                  }
            }
      }
#endif


#ifdef VIDEO_FILE
   int r_cnt,w_cnt;
    int ret;
    //char *buf1 = new char[PACKAGE];

      for(;;)
	{
	      r_cnt = fread(data,sizeof(char),PACKAGE,g_txvideo);

	      if(r_cnt > 0)
            {
        	      w_cnt = 0;
                  do{
                        if (UDT::ERROR == (ret = UDT::send(client, data + w_cnt, r_cnt - w_cnt, 0)))
                        {
                              cout << "send:" << UDT::getlasterror().getErrorMessage() << endl;
                              break;
                        }
                        //ret = write(serial.fd[0], &data[w_cnt],r_cnt - w_cnt);	
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
                  printf("----------------read_finish--------------\n");
                  break;
                  //lseek(tx_vedio, 0, SEEK_SET);
            }
    }

#endif
//    for (int i = 0; i < 1000000; i ++)
//    {
//       int ssize = 0;
//       int ss;
//       while (ssize < size)
//       {
//          if (UDT::ERROR == (ss = UDT::send(client, data + ssize, size - ssize, 0)))
//          {
//             cout << "send:" << UDT::getlasterror().getErrorMessage() << endl;
//             break;
//          }

//          ssize += ss;
//       }

//       if (ssize < size)
//          break;
//    }

   UDT::close(client);
   //delete [] data;
   return 0;
}

#ifndef WIN32
void* monitor(void* s)
#else
DWORD WINAPI monitor(LPVOID s)
#endif
{
   UDTSOCKET u = *(UDTSOCKET*)s;

   UDT::TRACEINFO perf;

   cout << "SendRate(Mb/s)\tRTT(ms)\tCWnd\tPktSndPeriod(us)\tRecvACK\tRecvNAK" << endl;

   while (true)
   {
      #ifndef WIN32
         sleep(1);
      #else
         Sleep(1000);
      #endif

      if (UDT::ERROR == UDT::perfmon(u, &perf))
      {
         cout << "perfmon: " << UDT::getlasterror().getErrorMessage() << endl;
         break;
      }

      cout << perf.mbpsSendRate << "\t\t" 
           << perf.msRTT << "\t" 
           << perf.pktCongestionWindow << "\t" 
           << perf.usPktSndPeriod << "\t\t\t" 
           << perf.pktRecvACK << "\t" 
           << perf.pktRecvNAK << endl;
   }

   #ifndef WIN32
      return NULL;
   #else
      return 0;
   #endif
}
