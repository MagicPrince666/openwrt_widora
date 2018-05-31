/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// Copyright (c) 1996-2014, Live Networks, Inc.  All rights reserved
// A test program that demonstrates how to stream - via unicast RTP
// - various kinds of file on demand, using a built-in RTSP server.
// main program

#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>
#include "DD_H264VideoFileServerMediaSubsession.hh"
#include "CONSTANT.h"
#include "FetchData.hh"

#include <stdint.h>
#include <pthread.h>
#include <signal.h>
#include <signum.h>
#include <execinfo.h>
#include <unistd.h>  
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define FIFO_NAME "/tmp/set_bitrates"
#define BUF_SIZE 128
 
// /****crash handler begin******/
// static void _signal_handler(int signum)  
// {  
//     void *array[10];  
//     size_t size;  
//     char **strings;  
//     size_t i;  
  
//     signal(signum, SIG_DFL); /* 还原默认的信号处理handler */  
  
//     size = backtrace (array, 10);  
//     strings = (char **)backtrace_symbols (array, size);  
  
//     fprintf(stderr, "widebright received SIGSEGV! Stack trace:\n");  
//     for (i = 0; i < size; i++) {  
//         fprintf(stderr, "%d %s \n",i,strings[i]);  
//     }  
      
//     free (strings);  
//     exit(1);  
// }  



UsageEnvironment* video_env;

//視頻採集
void* video_thread_func(void* param)
{
	TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  	video_env = BasicUsageEnvironment::createNew(*scheduler);
	ServerMediaSession* s = (ServerMediaSession*)param;

   s->addSubsession(DD_H264VideoFileServerMediaSubsession
		       ::createNew(*video_env, NULL));

   video_env->taskScheduler().doEventLoop(); // does not return
   return NULL;
}



//在线设置
void* set_thread_func(void* param)
{
  char buf[BUF_SIZE];
  memset(buf, 0, BUF_SIZE);
  int pipe_fd;
  //int res;
  int bytes_read = 0;

  if((mkfifo(FIFO_NAME,O_CREAT|O_EXCL)<0)&&(errno!=EEXIST))//创建有名管道,并设置相应的权限
    printf("cannot create fifoserver \n");
  printf("perparing for reading bytes ... \n");

  printf("PID %d open pipe O_RDONLY\n", getpid());
  pipe_fd = open(FIFO_NAME, O_RDONLY|O_NONBLOCK);
  if(pipe_fd <= 0)pthread_exit(NULL);
  else printf("start pipe read\n");

  while(1)
  {
    if (pipe_fd != -1) {
      bytes_read = read(pipe_fd, buf, sizeof(buf));
      if(bytes_read > 0)
      {
        printf("buf:%s\n",buf);
        char *str = strstr(buf,"AT+RATE=");
        if(str != NULL)
        {
           int rate = atoi(buf+8);
           printf("set bitrate:%d bps\n",rate);
           if(rate >= 1000000 && rate <= 16000000)//用此接口支持动态码率
           {       
            if(-1 == FetchData::bit_rate_setting(rate))
              printf("set bit rate error\n"); 
           }
           else printf("Parameter error!\nNo change\n");      	    
        }    
      }
    }
    
    sleep(1);
  }
  close(pipe_fd); 
  pthread_exit(NULL);
}

static pthread_t set_thread;
static pthread_t video_thread;

void createVideoEventLoop(ServerMediaSession* s)
{
    int res = pthread_create(&video_thread, NULL, video_thread_func,s); 
    if (res != 0)  
    {  
        printf(" VIDEO EVENT LOOP ERROR \n");  
        exit(EXIT_FAILURE);  
    }  
}

UsageEnvironment* env;

static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,
			   char const* streamName, char const* inputFileName, const char* inputAudioFileName); // fwd

int fps = 30;
int bitrates = 4096;
int port = 554;
char* vid = (char *)"/dev/video1";

static void help(char *s);

int main(int argc, char** argv) {

  // int init_cycle_buffer(void);
  // signal(SIGPIPE, _signal_handler);    // SIGPIPE，管道破裂。
  // signal(SIGSEGV, _signal_handler);    // SIGSEGV，非法内存访问
  // signal(SIGFPE, _signal_handler);       // SIGFPE，数学相关的异常，如被0除，浮点溢出，等等
  // signal(SIGABRT, _signal_handler);     // SIGABRT，由调用abort函数产生，进程非正常退出

  //RTSPServer videodev(video driver device)  audiodev(audio driver device) fps(>0) bitrates(unit:kbps >0) RTSPPORT(default 554)
  //eg: .RTSPServer /dev/video1 30  1024 554;
  // you can input "RTSPServer" instead of "RTSPServer /dev/video1  /dev/dsp 30  1024 554" 
  

  if(argc == 2 && strcmp(argv[1], "-h") == 0)
  {
    help(argv[0]);
    return 0;
  }

  if(argc == 1)
  {
      setVideoDevice(vid);
      setPort(port);
      setBitRate(bitrates);
      setFps(fps);
  }else
  {
     if(argc < 4)
      {
        help(argv[0]);
        return 0;
      }
      setVideoDevice(argv[1]);

      fps = atoi(argv[2]);
      if(fps <= 0)
      {
        help(argv[0]);
        return 0;
      }

      bitrates = atoi(argv[3]);
      if(bitrates <= 0)
      {
        help(argv[0]);
        return 0;
      }

      if(argc > 4)
      {
        int p = atoi(argv[4]);
        if(p > 0)
        {
          port = p;
        }
      }

	setPort(port);
	setBitRate(bitrates);
	setFps(fps);
  }

 
  // Begin by setting up our usage environment:
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  env = BasicUsageEnvironment::createNew(*scheduler);

  UserAuthenticationDatabase* authDB = NULL;
#ifdef ACCESS_CONTROL
  // To implement client access control to the RTSP server, do the following:
  authDB = new UserAuthenticationDatabase;
  authDB->addUserRecord("magic", "prince"); // replace these with real strings
  // Repeat the above with each <username>, <password> that you wish to allow
  // access to the server.
#endif

  // Create the RTSP server:
  RTSPServer* rtspServer = RTSPServer::createNew(*env, getPort(), authDB);
  if (rtspServer == NULL) {
    *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
    exit(1);
  }

  char const* streamName = "stream";
  char const* descriptionString
    = "Session streamed by \"DM RTSP\"";

  {

    ServerMediaSession* sms
      = ServerMediaSession::createNew(*env, streamName, streamName,
				      descriptionString);

    while(video_env == NULL)
    {
      createVideoEventLoop(sms);
      sleep(1);
    }

    pthread_create(&set_thread, NULL, set_thread_func, NULL);

    rtspServer->addServerMediaSession(sms);

    announceStream(rtspServer, sms, streamName, NULL,NULL);
  }

  // Also, attempt to create a HTTP server for RTSP-over-HTTP tunneling.
  // Try first with the default HTTP port (80), and then with the alternative HTTP
  // port numbers (8000 and 8080).

  if (rtspServer->setUpTunnelingOverHTTP(80) || rtspServer->setUpTunnelingOverHTTP(8000) || rtspServer->setUpTunnelingOverHTTP(8080)) {
    *env << "\n(We use port " << rtspServer->httpServerPortNum() << " for optional RTSP-over-HTTP tunneling.)\n";
  } else {
    *env << "\n(RTSP-over-HTTP tunneling is not available.)\n";
  }

  env->taskScheduler().doEventLoop(); // does not return

  return 0; // only to prevent compiler warning
}

static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,
			   char const* streamName, char const* inputFileName, const char* inputAudioFileName) {

  char* url = rtspServer->rtspURL(sms);
  UsageEnvironment& env = rtspServer->envir();
  env << "\n\"" << streamName << "\" stream, from the Video file \""
      << inputFileName << ", from the Audio file \"\n"
      << inputAudioFileName << ",\"\n";
  env << "Play this stream using the URL \"" << url << "\"\n";
  delete[] url;
}

static void help(char *s)
{
  printf("====================================\n");
  printf("CMD tips:\n");
  printf("  %s videodev(video driver device) fps(>0) bitrates(unit:kbps >0) RTSPPORT(default 554)\n",s);
  printf("  eg: %s /dev/video1 30 1024 554\n",s);
  printf("  you can input \"%s\" instead of \"%s %s %d %d %d\" \n",s,s,vid,fps,bitrates,port);
  printf("  use \"echo AT+RATE=2000000 > %s\" to change bitrates by real time\n",FIFO_NAME);
  printf("====================================\n");
  return;
}
