#CXX = /Volumes/WPJ342/chaos_calmer/staging_dir/toolchain-mips_34kc_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mips-openwrt-linux-uclibc-g++
#export STAGING_DIR = /Volumes/WPJ342/chaos_calmer/openwrt/staging_dir/toolchain-mips_34kc_gcc-4.8-linaro_uClibc-0.9.33.2/bin/

TARGET	= rtsp_server

livedir := live/
livemedia := $(livedir)liveMedia
usageenviroment := $(livedir)UsageEnvironment
basicusage := $(livedir)BasicUsageEnvironment
groupsock := $(livedir)groupsock

LIBDIR = -L$(usageenviroment) -L$(basicusage) -L$(livemedia) -L$(groupsock)

DIR		= ./release ./release/src
INC		= -I./release/Head -I$(usageenviroment)/include -I$(basicusage)/include -I$(livemedia)/include -I$(groupsock)/include
CFLAGS	= -g -Wall -O3
LDFLAGS = $(LIBDIR) -lliveMedia -lBasicUsageEnvironment -lgroupsock -lUsageEnvironment -lpthread -ldl

OBJPATH	= .

FILES	= $(foreach dir,$(DIR),$(wildcard $(dir)/*.cpp))

OBJS	= $(patsubst %.cpp,%.o,$(FILES))

all:
	cd $(livedir) ; ./genMakefiles mips
	cd $(livemedia) ; $(MAKE) 
	cd $(groupsock) ; $(MAKE) 
	cd $(usageenviroment) ; $(MAKE) 
	cd $(basicusage) ; $(MAKE) 
	cd . ; $(MAKE) $(OBJS) $(TARGET)

$(OBJS):%.o:%.cpp
	$(CXX) $(CFLAGS) $(INC) -c -o $(OBJPATH)/$(notdir $@) $< 

$(TARGET):$(OBJPATH)
	$(CXX) -o $@ $(OBJPATH)/*.o $(LDFLAGS)

$(OBJPATH):
	mkdir -p $(OBJPATH)

clean:
	-rm -f $(OBJPATH)/*.o
	-rm -f $(TARGET)
	cd $(livemedia) ; $(MAKE) clean
	cd $(groupsock) ; $(MAKE) clean
	cd $(usageenviroment) ; $(MAKE) clean
	cd $(basicusage) ; $(MAKE) clean