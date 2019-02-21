#CROSS_COMPILE := /Volumes/WPJ342/opxlinkhs/staging_dir/toolchain-mips_24kc_gcc-7.3.0_musl/bin/mips-openwrt-linux-musl-
#CXX := $(CROSS_COMPILE)g++
#CC := $(CROSS_COMPILE)gcc
TARGET	= uvc_aoa

DIR		= . ./H264_camera
INC		= -I. -I./H264_camera
CFLAGS	= -g -Wall

OBJPATH	= .

LDFLAGS := -lusb-1.0 -lpthread

FILES	= $(foreach dir,$(DIR),$(wildcard $(dir)/*.cpp))
CFILES	= $(foreach dir,$(DIR),$(wildcard $(dir)/*.c))

OBJS	= $(patsubst %.cpp,%.o,$(FILES))
COBJS	= $(patsubst %.c,%.o,$(CFILES))

all:$(OBJS) $(COBJS) $(TARGET)

$(OBJS):%.o:%.cpp
	$(CXX) $(CFLAGS) $(INC) -c -o $(OBJPATH)/$(notdir $@) $< 

$(COBJS):%.o:%.c
	$(CC) $(CFLAGS) $(INC) -c -o $(OBJPATH)/$(notdir $@) $< 

$(TARGET):$(OBJPATH)
	$(CC) -o $@ $(OBJPATH)/*.o $(LDFLAGS)

clean:
	-rm -f $(OBJPATH)/*.o
	-rm -f $(TARGET)