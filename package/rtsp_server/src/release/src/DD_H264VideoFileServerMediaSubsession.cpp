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
// "liveMedia"
// Copyright (c) 1996-2014 Live Networks, Inc.  All rights reserved.
// A 'ServerMediaSubsession' object that creates new, unicast, "RTPSink"s
// on demand, from a H264 video file.
// Implementation

#include "DD_H264VideoFileServerMediaSubsession.hh"
#include "H264VideoRTPSink.hh"
#include "Tiam335xH264Source.hh"
#include "H264VideoStreamDiscreteFramer.hh"


# define __DBGFUNS
# ifdef __DBGFUNS
# define DBGFUNS(fmt,args...) fprintf(stdout,  fmt,  ##args)
# else
# define DBGFUNS(fmt,args...)
# endif

DD_H264VideoFileServerMediaSubsession*
DD_H264VideoFileServerMediaSubsession::createNew(UsageEnvironment & env, FramedSource * source) {
 OutPacketBuffer::maxSize = 521366;
  return new DD_H264VideoFileServerMediaSubsession(env, source);
}

DD_H264VideoFileServerMediaSubsession
::DD_H264VideoFileServerMediaSubsession(UsageEnvironment & env, FramedSource * source)
  : OnDemandServerMediaSubsession(env, true)
{

     m_pSource = source;  
     m_pSDPLine = 0;
}

DD_H264VideoFileServerMediaSubsession::~DD_H264VideoFileServerMediaSubsession() 
{
  DBGFUNS("__DD_H264VideoFileServerMediaSubsession::destructor__\n");
   if (m_pSDPLine)  
    {  
        free(m_pSDPLine);  
        m_pSDPLine=NULL;
    }  

}
#if 0
#endif

FramedSource* DD_H264VideoFileServerMediaSubsession::createNewStreamSource(
  unsigned /*clientSessionId*/, unsigned& estBitrate) 
  
  {

  DBGFUNS("__DD_H264VideoFileServerMediaSubsession::createNewStreamSource__\n");
  estBitrate = 1024000; // kbps, estimate

  //视频真正实现类
  return H264VideoStreamDiscreteFramer::createNew(envir(), Tiam335xH264Source::createNew(envir()));
}

RTPSink* DD_H264VideoFileServerMediaSubsession
::createNewRTPSink(Groupsock* rtpGroupsock,
		   unsigned char rtpPayloadTypeIfDynamic,
		   FramedSource* /*inputSource*/) {
 DBGFUNS("__DD_H264VideoFileServerMediaSubsession::createNewRTPSink__\n");
  return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}


void DD_H264VideoFileServerMediaSubsession::afterPlayingDummy(void* ptr)
{
     DD_H264VideoFileServerMediaSubsession * This = (DD_H264VideoFileServerMediaSubsession *)ptr;  
    This->m_done = 0xff;
}
void DD_H264VideoFileServerMediaSubsession::chkForAuxSDPLine(void* ptr)
{
   DD_H264VideoFileServerMediaSubsession * This = (DD_H264VideoFileServerMediaSubsession *)ptr;  
    This->chkForAuxSDPLine1();
}
  void DD_H264VideoFileServerMediaSubsession::chkForAuxSDPLine1()
  {

    printf("chkForAuxSDPLine1 1 \n");
    if (m_pDummyRTPSink->auxSDPLine())  
    {  
      printf("chkForAuxSDPLine1 2 \n");
        m_done = 0xff;  
    }  else
    {
       char const* fmtpFmt =
      "a=fmtp:%d packetization-mode=1"
      ";profile-level-id=000000"
      ";sprop-parameter-sets=H264\r\n";

    unsigned fmtpFmtSize = strlen(fmtpFmt)+3/* max char len */;

    char* fmtp = new char[fmtpFmtSize];
    sprintf(fmtp,fmtpFmt,m_pDummyRTPSink->rtpPayloadType());

    delete[] m_pSDPLine;
    m_pSDPLine = fmtp;//????????SDP???
    printf("SDP Info:%s\n",m_pSDPLine);
    
    }
    // else  
    // {  printf("chkForAuxSDPLine1 3 \n");
    //     double delay = 1000.0 / (FRAME_PER_SEC * 2);  // ms  
    //     int to_delay = delay * 1000;  // us  
  
    //     nextTask() = envir().taskScheduler().scheduleDelayedTask(to_delay, chkForAuxSDPLine, this);  
    // }  
  }

char const * DD_H264VideoFileServerMediaSubsession::getAuxSDPLine(RTPSink * rtpSink, FramedSource * inputSource)
{
   if (m_pSDPLine)  
    {  
        return m_pSDPLine;  
    }  
  
    m_pDummyRTPSink = rtpSink;  
  printf("getAuxSDPLine 1 \n");
    //mp_dummy_rtpsink->startPlaying(*source, afterPlayingDummy, this);  
    // m_pDummyRTPSink->startPlaying(*inputSource, 0, 0);  
  printf("getAuxSDPLine 3 \n");
  
    // chkForAuxSDPLine(this);  
  printf("getAuxSDPLine 4 \n");
  
    // m_done = 0;  
  
    // envir().taskScheduler().doEventLoop(&m_done);  
    printf("getAuxSDPLine 5 m_done:%d \n",m_done);
    char const* fmtpFmt =
      "a=fmtp:%d packetization-mode=1"
      ";profile-level-id=000000"
      ";sprop-parameter-sets=H264\r\n";

    unsigned fmtpFmtSize = strlen(fmtpFmt)+3/* max char len */;

    char* fmtp = new char[fmtpFmtSize];
    sprintf(fmtp,fmtpFmt,m_pDummyRTPSink->rtpPayloadType());

    m_pSDPLine = fmtp;//????????SDP???
    printf("SDP Info:%s\n",m_pSDPLine);
    // m_pSDPLine = strdup(m_pDummyRTPSink->auxSDPLine());  
  
    // m_pDummyRTPSink->stopPlaying();  
    printf("getAuxSDPLine 6 \n");
  
    return m_pSDPLine;  
} 
