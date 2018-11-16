#CROSS = /Volumes/WPJ342/opxlinkhs/staging_dir/toolchain-mips_24kc_gcc-7.3.0_musl/bin/mips-openwrt-linux-musl-
#CXX = $(CROSS)g++
#CC = $(CROSS)gcc

#export CC CXX CROSS

TARGET1	= rtsp_server
TARGET2	= rtsp_client

#CLIENTDIR = client

livedir := live/
livemedia := $(livedir)liveMedia
usageenviroment := $(livedir)UsageEnvironment
basicusage := $(livedir)BasicUsageEnvironment
groupsock := $(livedir)groupsock
testProgs := $(livedir)testProgs

LIBDIR = -L$(usageenviroment) -L$(basicusage) -L$(livemedia) -L$(groupsock)

DIR		= ./client ./release/src
INC		= -I./client -I./release/Head -I$(usageenviroment)/include -I$(basicusage)/include -I$(livemedia)/include -I$(groupsock)/include
CFLAGS	= -g -Wall -O3
LDFLAGS = $(LIBDIR) -lliveMedia -lBasicUsageEnvironment -lgroupsock -lUsageEnvironment -lpthread -ldl

OBJPATH	= .

FILES	= $(foreach dir,$(DIR),$(wildcard $(dir)/*.cpp))
CFILES	= $(foreach dir,$(DIR),$(wildcard $(dir)/*.c))

OBJS	= $(patsubst %.cpp,%.o,$(FILES))
COBJS	= $(patsubst %.c,%.o,$(CFILES))

all:
	cd $(livedir) ; ./genMakefiles mips
	cd $(livemedia) ; $(MAKE) 
	cd $(groupsock) ; $(MAKE) 
	cd $(usageenviroment) ; $(MAKE) 
	cd $(basicusage) ; $(MAKE) 
#	cd $(testProgs) ; $(MAKE)
#	cd $(CLIENTDIR) ; $(MAKE)
	cd . ; $(MAKE) $(OBJS) $(COBJS) $(TARGET2) $(TARGET1)


$(OBJS):%.o:%.cpp
	$(CXX) $(CFLAGS) $(INC) -c -o $(OBJPATH)/$(notdir $@) $< 

$(COBJS):%.o:%.c
	$(CC) $(CFLAGS) $(INC) -c -o $(OBJPATH)/$(notdir $@) $< 

$(TARGET1):$(OBJPATH)
	$(CXX) -o $@ CONSTANT.o DD_H264VideoFileServerMediaSubsession.o FetchData.o Tiam335xH264Source.o \
	h264_xu_ctrls.o testOnDemandRTSPServer.o thread.o v4l2uvc.o $(LDFLAGS) 

$(TARGET2):$(OBJPATH)
	$(CXX) -o $@ a2spipe.o accessory.o aoaproxy.o ringbuffer.o serial.o testRTSPClient.o $(LDFLAGS) -lusb-1.0

$(OBJPATH):
	mkdir -p $(OBJPATH)

clean:
	-rm -f $(OBJPATH)/*.o
	-rm -f $(TARGET1) $(TARGET2)
	cd $(livemedia) ; $(MAKE) clean
	cd $(groupsock) ; $(MAKE) clean
	cd $(usageenviroment) ; $(MAKE) clean
	cd $(basicusage) ; $(MAKE) clean
	#cd $(CLIENTDIR) ; $(MAKE) clean

